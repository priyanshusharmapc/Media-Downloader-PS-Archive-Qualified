/*
 *
 *  Copyright (c) 2022
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

#pragma once

#include <QTimer>
#include <QObject>

#include <type_traits>
#include <functional>

namespace utils{
	namespace qtimer
	{
		namespace details{
			#if __cplusplus >= 201703L
				template< typename Function,typename ... Args >
				using result_of = std::invoke_result_t< Function,Args ... > ;
			#else
				template< typename Function,typename ... Args >
				using result_of = std::result_of_t< Function( Args ... ) > ;
			#endif
		}

		/*
		 * Function must take an int and must return bool
		 */
		template< typename Function,
			  typename std::enable_if< std::is_same< details::result_of< Function,int >,bool >::value,int >::type = 0 >
		void run( int interval,Function&& function )
		{
			class Timer{
			public:
				Timer( int interval,Function&& function ) :
					m_function( std::forward< Function >( function ) )
				{
					auto timer = new QTimer() ;

					QObject::connect( timer,&QTimer::timeout,[ timer,this ](){

						m_counter++ ;

						if( m_function( m_counter ) ){

							timer->stop() ;

							timer->deleteLater() ;

							delete this ;
						}
					} ) ;

					timer->start( interval ) ;
				}
			private:
				int m_counter = 0 ;
				Function m_function ;
			} ;

			new Timer( interval,std::forward< Function >( function ) ) ;
		}


		/*
		 * Owner-aware one-shot timer. The timer is parented to the supplied
		 * QObject, so destruction of the real owner cancels the delayed callback
		 * instead of leaving a raw-owner closure alive in the event queue.
		 */
		template< typename Function,
			  typename std::enable_if< std::is_void< details::result_of< Function > >::value,int >::type = 0 >
		void run( QObject * context,int interval,Function&& function )
		{
			if( !context )return ;

			// Store the callback directly in a QObject-owned timer rather than in
			// a Qt functor slot. This preserves move-only callback support on the
			// Qt 5.10 compatibility floor while still making owner destruction an
			// unconditional cancellation boundary.
			class Timer : public QTimer
			{
			public:
				Timer( QObject * owner,int timeout,Function&& callback ) :
					QTimer( owner ),m_function( std::move( callback ) )
				{
					this->setSingleShot( true ) ;
					QObject::connect( this,&QTimer::timeout,this,&Timer::fire ) ;
					this->start( timeout ) ;
				}
			private:
				void fire()
				{
					m_function() ;
					this->deleteLater() ;
				}
				Function m_function ;
			} ;

			new Timer( context,interval,std::forward< Function >( function ) ) ;
		}

		/*
		 * Function must takes no argument and will be called once when the interval pass
		 */
		template< typename Function,
			  typename std::enable_if< std::is_void< details::result_of< Function > >::value,int >::type = 0 >
		void run( int interval,Function&& function )
		{
			run( interval,[ function = std::forward< Function >( function ) ]( int s )mutable{

				Q_UNUSED( s )

				function() ;

				return true ;
			} ) ;
		}
	}
}
