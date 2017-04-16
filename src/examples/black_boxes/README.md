# Black Boxes

```bash
# Configuration
DB_USER="username"
DB_PASS="password"
DB_NAME="the_db"
SERVER_ADDR="tcp://0.0.0.0:9000"
CLIENT_ADDR="tcp://0.0.0.0:9001"

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
```
