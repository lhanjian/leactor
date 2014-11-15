#ifndef NGX_PARSE
#define NGX_PARSE
int ngx_http_parse_request_line(request_t *, lt_buffer_t *);
int ngx_http_parse_header_line(request_t *, lt_buffer_t *, int);
int ngx_http_parse_status_line(request_t *, lt_buffer_t *, http_status_t *);
#endif
