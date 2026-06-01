#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zlm_handle_t zlm_handle_t;

// Create / destroy wrapper
zlm_handle_t* zlm_create(void);
void zlm_destroy(zlm_handle_t* h);

// Start server using explicit ini + ssl paths (nullable).
// Returns 0 on success (thread started), non-zero on immediate error.
int zlm_start_with_paths(zlm_handle_t* h,
                         const char* ini_path /*nullable*/,
                         const char* ssl_pem_path /*nullable*/);

// Stop server (blocks until shutdown finished)
int zlm_stop(zlm_handle_t* h);

// Set listen IP and port. ip may be nullptr to use configuration/defaults.
// If server is running, wrapper will restart listen sockets to apply change.
int zlm_set_listen(zlm_handle_t* h, const char* ip /*nullable*/, uint16_t port);

// Query runtime
int zlm_is_running(zlm_handle_t* h);

#ifdef __cplusplus
}
#endif
