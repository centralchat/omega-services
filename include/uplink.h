#ifndef __UPLINK_H__
#define __UPLINK_H__

E int  uplink_connect         (void);
E void uplink_write_callback  (socket_t *);
E void uplink_error_callback  (socket_t *);
E void uplink_read_callback   (socket_t *);
E void uplink_set             (server_t *);

#endif