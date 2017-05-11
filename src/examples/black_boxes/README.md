# Black Boxes

```bash
# Configuration
DB_USER="username"
DB_PASS="password"
DB_NAME="the_db"
SERVER_ADDR="tcp://0.0.0.0:9000"
CLIENT_ADDR="tcp://0.0.0.0:9001"
REDIS_ADDR=localhost
REDIS_PORT=6379

# Primality.
GLOG_logtostderr=1 ./build/examples_black_boxes_primality_server \
    $DB_USER $DB_PASS $DB_NAME $SERVER_ADDR
GLOG_logtostderr=1 ./build/examples_black_boxes_primality_client \
    $DB_USER $DB_PASS $DB_NAME $SERVER_ADDR $CLIENT_ADDR

# Key Value.
GLOG_logtostderr=1 ./build/examples_black_boxes_key_value_server \
    $DB_USER $DB_PASS $DB_NAME $SERVER_ADDR
GLOG_logtostderr=1 ./build/examples_black_boxes_key_value_client \
    $DB_USER $DB_PASS $DB_NAME $SERVER_ADDR $CLIENT_ADDR

# Redis.
GLOG_logtostderr=1 ./build/examples_black_boxes_redis_server \
    $DB_USER $DB_PASS $DB_NAME $REDIS_ADDR $REDIS_PORT $SERVER_ADDR
GLOG_logtostderr=1 ./build/examples_black_boxes_redis_client \
    $DB_USER $DB_PASS $DB_NAME $SERVER_ADDR tcp://0.0.0.0:9001 larry
GLOG_logtostderr=1 ./build/examples_black_boxes_redis_client \
    $DB_USER $DB_PASS $DB_NAME $SERVER_ADDR tcp://0.0.0.0:9002 moe
GLOG_logtostderr=1 ./build/examples_black_boxes_redis_client \
    $DB_USER $DB_PASS $DB_NAME $SERVER_ADDR tcp://0.0.0.0:9003 curly
```