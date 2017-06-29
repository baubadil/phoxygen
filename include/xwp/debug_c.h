
#ifndef XWP_DEBUG_C_H
#define XWP_DEBUG_C_H

#ifdef  __cplusplus
extern "C" {
#endif

void DebugEnter(const char *pcszFormat, ...);
void DebugLeave(const char *pcszFormat, ...);
void DebugLog(const char *pcszFormat, ...);

#ifdef  __cplusplus
} /* extern "C" */
#endif

#endif /* XWP_DEBUG_C_H */
