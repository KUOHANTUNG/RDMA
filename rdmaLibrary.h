#ifndef RDMA_COMMON_H
#define RDMA_COMMON_H

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rdma/rdma_cma.h>

#define TEST_NZ(x) do { if ( (x)) die("error: " #x " failed (returned non-zero)." ); } while (0)
#define TEST_Z(x)  do { if (!(x)) die("error: " #x " failed (returned zero/null)."); } while (0)
struct message {
  enum {
    MSG_MR,
    MSG_DONE
  } type;

  union {
    struct ibv_mr mr;
  } data;
};
/**build up a fertile context base on ibv_context */
struct context {
  struct ibv_context *ctx;//device conetext
  struct ibv_pd *pd;//protext domain
  struct ibv_cq *cq;//complete queue
  struct ibv_comp_channel *comp_channel;// complete channel using for send complete event(notice) 
  pthread_t cq_poller_thread;//the thread processing the completion event
};

struct connection {
  struct rdma_cm_id *id;
  struct ibv_qp *qp;

  int connected;

  struct ibv_mr *recv_mr;
  struct ibv_mr *send_mr;
  struct ibv_mr *rdma_local_mr;
  struct ibv_mr *rdma_remote_mr;

  struct ibv_mr peer_mr;

  struct message *recv_msg;
  struct message *send_msg;

  char *rdma_local_region;
  char *rdma_remote_region;

  enum {
    SS_INIT,
    SS_MR_SENT_META,
    SS_MR_SENT_DATA,
    SS_DONE_SENT
  } send_state;// all thouse flags are not from WC but are just to tell the information to follow the current connection state

  enum {
    RS_INIT,
    RS_MR_RECV_META,
    RS_MR_RECV_DATA,
    RS_DONE_RECV
  } recv_state;
};
enum mode {
  M_WRITE,
  M_READ
};

void die(const char *reason);

void build_connection(struct rdma_cm_id *id);
void build_params(struct rdma_conn_param *params);
void destroy_connection(void *context);
void * get_local_message_region(void *context);
void on_connect(void *context);
void send_mr(void *context);
void set_mode(enum mode m);

void build_context(struct ibv_context *verbs);
void build_qp_attr(struct ibv_qp_init_attr *qp_attr);
char * get_peer_message_region(struct connection *conn);
void on_completion(struct ibv_wc *);
void * poll_cq(void *);
void post_receives(struct connection *conn);
void register_memory(struct connection *conn);
void send_message(struct connection *conn);
int on_addr_resolved(struct rdma_cm_id *id, char* message);
int on_connection(struct rdma_cm_id *id);
int on_disconnect(struct rdma_cm_id *id);
int on_connect_request(struct rdma_cm_id *id);
int on_connection_server(struct rdma_cm_id *id);
int on_disconnect_server(struct rdma_cm_id *id);
void on_completion_server(struct ibv_wc *wc);
void send_data(struct connection *conn);
#endif
