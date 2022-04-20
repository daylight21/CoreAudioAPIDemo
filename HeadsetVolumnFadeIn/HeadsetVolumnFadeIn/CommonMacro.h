#pragma once

#define CHECK_HR_RETURN(hr, state, str) \
if (hr != S_OK) { \
    LOG_ERROR(str); \
    return state; \
}

#define CHECK_HR_CONTINUE(hr, str) \
if (hr != S_OK) { \
    LOG_ERROR(str); \
    continue; \
}

#define CHECK_NULLPTR_RETURN(ptr, state, str) \
if (ptr == nullptr) { \
    LOG_ERROR(str); \
    return state; \
}

#define CHECK_NULLPTR_CONTINUE(ptr, str) \
if (ptr == nullptr) { \
    LOG_ERROR(str); \
    continue; \
}

#define CHECK_HR_AND_NULLPTR_RETURN(hr, ptr, state, str) \
if (hr != S_OK || ptr == nullptr) { \
    LOG_ERROR(str); \
    return state; \
}

#define CHECK_HR_AND_NULLPTR_CONTINUE(hr, ptr, str) \
if (hr != S_OK || ptr == nullptr) { \
    LOG_ERROR(str); \
    continue; \
}