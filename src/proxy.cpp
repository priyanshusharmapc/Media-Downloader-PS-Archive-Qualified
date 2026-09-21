/*
 *
 *  Copyright (c) 2023
 *  name : Francis Banyikwa
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "proxy.h"
#include "utils/qprocess.hpp"
#include "utils/threads.hpp"
#include "utility.h"

#include <atomic>
#include <limits>

static QString _find_proxy( const QProcessEnvironment& )
{
	return {} ;
}

template< typename ... Args >
static QString _find_proxy( const QProcessEnvironment& env,const char * first,Args&& ... rest )
{
	auto m = env.value( first ) ;

	if( m.isEmpty() ){

		return _find_proxy( env,std::forward< Args >( rest ) ... ) ;
	}else{
		return m ;
	}
}

static QString _proxy_find( const Context& ctx )
{
	const auto& env = ctx.Engines().processEnvironment() ;

	return _find_proxy( env,"all_proxy","ALL_PROXY","https_proxy","http_proxy","HTTPS_PROXY","HTTP_PROXY" ) ;
}

static QByteArray _hex_to_decimal( const QByteArray& mm )
{
	auto m = mm.toLower() ;

	auto a = m[ 0 ] ;
	auto b = m[ 1 ] ;

	int r ;

	if( a < 'a' ){

		r = 16 * ( a - '0' ) ;
	}else{
		r = 16 * ( a - 'a' + 10 ) ;
	}

	if( b < 'a' ){

		r += b - '0' ;
	}else{
		r += b - 'a' + 10 ;
	}

	return QByteArray::number( r ) ;
}

static QByteArray _ip_address( const QByteArray& e )
{
	auto a = _hex_to_decimal( e.mid( 0,2 ) ) ;
	auto b = _hex_to_decimal( e.mid( 2,2 ) ) ;
	auto c = _hex_to_decimal( e.mid( 4,2 ) ) ;
	auto d = _hex_to_decimal( e.mid( 6,2 ) ) ;

	return d + "." + c + "." + b + "." + a ;
}

static void _get_proxy_from_gateway_linux( Context& ctx,const QByteArray& addr,bool firstTime )
{
	QFile file( "/proc/net/route" ) ;

	if( !file.open( QIODevice::ReadOnly ) ){
		ctx.setNetworkProxy( firstTime ) ;
		return ;
	}

	QByteArray bestGateway ;
	int bestMetric = std::numeric_limits< int >::max() ;

	for( const auto& line : util::split( file.readAll(),'\n' ) ){
		const auto fields = line.simplified().split( ' ' ) ;
		if( fields.size() < 7 || fields[ 1 ] != "00000000" ){
			continue ;
		}

		bool gatewayOk = false ;
		bool flagsOk = false ;
		bool metricOk = false ;
		fields[ 2 ].toUInt( &gatewayOk,16 ) ;
		const auto flags = fields[ 3 ].toUInt( &flagsOk,16 ) ;
		const auto metric = fields[ 6 ].toInt( &metricOk,10 ) ;

		// Linux route flags: RTF_UP=0x1, RTF_GATEWAY=0x2.
		if( !gatewayOk || !flagsOk || !metricOk || metric < 0 ||
		    fields[ 2 ].size() != 8 || ( flags & 0x3u ) != 0x3u ){
			continue ;
		}

		if( metric < bestMetric ){
			bestMetric = metric ;
			bestGateway = fields[ 2 ] ;
		}
	}

	if( bestGateway.isEmpty() ){
		ctx.setNetworkProxy( firstTime ) ;
		return ;
	}

	QString expanded = addr ;
	expanded.replace( "${gateway}",_ip_address( bestGateway ) ) ;
	ctx.setNetworkProxy( expanded,firstTime ) ;
}

static void _get_proxy_from_gateway_win( Context& ctx,const QByteArray& addr,bool firstTime )
{
	auto m = utility::windowsGateWayAddress() ;

	if( m.isEmpty() ){

		ctx.setNetworkProxy( firstTime ) ;
	}else{
		QString s = addr ;

		s.replace( "${gateway}",m ) ;

		ctx.setNetworkProxy( s,firstTime ) ;
	}
}

using mm = settings::proxySettings ;

static std::atomic< quint64 > _proxyGeneration{ 0 } ;

void proxy::set( Context& ctx,bool firstTime,const QByteArray& proxyAddress,const mm::type& m )
{
	const auto generation = ++_proxyGeneration ;

	if( utility::platformIsWindows() && m.system() ){
		class systemProxyLookup
		{
		public:
			systemProxyLookup( Context& context,bool first,quint64 generation ) :
				m_ctx( &context ),m_firstTime( first ),m_generation( generation )
			{
			}
			QList< QNetworkProxy > bg()
			{
				return QNetworkProxyFactory::systemProxyForQuery() ;
			}
			void fg( QList< QNetworkProxy >&& proxies )
			{
				if( m_generation != _proxyGeneration.load() ){
					return ;
				}

				// Qt returns alternatives in preference order. Preserve that order,
				// including an explicit direct connection.
				for( const auto& proxy : proxies ){
					if( proxy.type() == QNetworkProxy::NoProxy ||
					    proxy.type() == QNetworkProxy::DefaultProxy ){
						m_ctx->setNetworkProxy( m_firstTime ) ;
						return ;
					}
					if( !proxy.hostName().isEmpty() ){
						m_ctx->setNetworkProxy( proxy,m_firstTime ) ;
						return ;
					}
				}
				m_ctx->setNetworkProxy( m_firstTime ) ;
			}
		private:
			Context * m_ctx ;
			bool m_firstTime ;
			quint64 m_generation ;
		} ;

		utils::qthread::run( &ctx.mainWidget(),systemProxyLookup( ctx,firstTime,generation ) ) ;
		return ;

	}else if( m.none() ){

		ctx.setNetworkProxy( firstTime ) ;

	}else if( m.env() ){

		ctx.setNetworkProxy( _proxy_find( ctx ),firstTime ) ;

	}else if( proxyAddress.contains( "${gateway}" ) ){

		if( utility::platformIsLinux() ){

			_get_proxy_from_gateway_linux( ctx,proxyAddress,firstTime ) ;

		}else if( utility::platformIsWindows() ){

			_get_proxy_from_gateway_win( ctx,proxyAddress,firstTime ) ;
		}else{
			ctx.setNetworkProxy( firstTime ) ;
		}
	}else{
		ctx.setNetworkProxy( proxyAddress,firstTime ) ;
	}
}
