#ifndef __UPLINK_H__
#define __UPLINK_H__

E int  uplink_connect         (void);
E void uplink_write_callback  (Socket *);
E void uplink_error_callback  (Socket *);
E void uplink_read_callback   (Socket *);
E void uplink_set             (server_t *);

#endif