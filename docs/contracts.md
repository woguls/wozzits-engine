# System Contracts

## Logging System
- Must be thread-safe
- Must use worker thread
- Must never block caller thread

## Threading System
- Must not expose raw OS threads to API
- Must use ThreadHandle abstraction only

## Window System
- Must not include Win32 headers in API
- Must return opaque handles only