# paho mqtt ping pong demo

This is paho-mqtt-cpp basic exmple, which demo basic ping pong.

You can use this program for the most basic stability test for mqtt.

## build

`conan build . --build=missing`

## execution

1. run `pong` first.
2. then run `ping`.

You can modify default configuration with file `config.toml` at current path in following config value (modify on your own).

```toml
BROKER_ADDRESS = "tcp://localhost:1883"
TEST_NAME = "pingpong"
PING_PERIOD_MS = 250
```

You can monitor all messages with topic `${TEST_NAME}/#`.
