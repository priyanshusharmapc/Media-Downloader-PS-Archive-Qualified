# Media Downloader PS UI/UX Mockups

> **AI-generated inspiration only. Not a final implementation specification.**

These mockups are visual references for the Media Downloader PS UI/UX overhaul. They are intended to guide presentation quality, layout hierarchy, information density, navigation, and usability.

They are **not** one-to-one implementation contracts. The final Qt/C++ implementation must remain grounded in the repository's actual features, backend behavior, state semantics, safety invariants, and existing workflows.

## Governing rules

- Existing downloader, Library, Configure, and About workflows keep their functional behavior unless a deliberate product change is separately approved.
- Home, Archive Data Explorer, and Settings/Portability may introduce carefully scoped functionality where explicitly approved.
- Archive Data Explorer must represent the real Archive structures and remain read-only for canonical state.
- Mock data, controls, statistics, navigation items, or database concepts shown in an AI image must not be implemented unless supported by the repository or deliberately approved.
- The images are motivation/reference material for UI quality, not a substitute for source-grounded design and testing.
- No "MDH" suffix should be added to the user-facing application title. Use **Media Downloader PS**.

## Canonical mockup set

1. Home
2. Basic Downloader
3. Batch Downloader
4. Playlist Downloader
5. Archive
6. Archive Data Explorer
7. Library
8. Settings & Portability
9. About

The PNG reference set is mirrored in Google Drive here:

https://drive.google.com/drive/folders/1v6rEYf1-S1h2D61u45NH_OUWe9tHyeCm

## Implementation authority

When a mock conflicts with current repository semantics, the repository and approved UI/UX implementation plan win. Any intentional functional extension should be documented and tested as a product change rather than inferred from an image.
