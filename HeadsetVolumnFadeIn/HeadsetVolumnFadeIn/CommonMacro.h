#pragma once

#define CHECK_HR_RETURN(hr, state, str) \
if (hr != S_OK) { \
    LOG_ERROR(str); \
    return state; \
}

#define CHECK_HR_BREAK(hr, str) \
if (hr != S_OK) { \
    LOG_ERROR(str); \
    break; \
}

#define CHECK_HR(hr, str) \
if (hr != S_OK) { \
    LOG_ERROR(str); \
    return; \
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

#define CHECK_NULLPTR(ptr, str) \
if (ptr == nullptr) { \
    LOG_ERROR(str); \
    return; \
}

#define CHECK_HR_AND_NULLPTR_RETURN(hr, ptr, state, str) \
if (hr != S_OK || ptr == nullptr) { \
    LOG_ERROR(str); \
        return state; \
}

#define CHECK_HR_AND_NULLPTR(hr, ptr, str) \
if (hr != S_OK || ptr == nullptr) { \
    LOG_ERROR(str); \
        return; \
}

#define CHECK_HR_AND_NULLPTR_CONTINUE(hr, ptr, str) \
if (hr != S_OK || ptr == nullptr) { \
    LOG_ERROR(str); \
    continue; \
}

#define CHECK_MAP_CONTAINS_KEY_RETURN(map, key) \
if (map.count(key) == 0) { \
    LOG_WARNING("map don't have key"); \
    return; \
}