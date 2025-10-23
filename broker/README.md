# Mosquitto MQTT Broker Configuration

This directory contains the configuration files and Docker deployment setup for the Eclipse Mosquitto MQTT broker used in the DATALOGGER system. The broker provides message routing between ESP32 gateway devices and web dashboard clients.

## Directory Structure

```
broker/
├── mosquitto.conf              # Main configuration file
├── config/
│   └── auth/
│       └── passwd.txt          # Bcrypt-hashed user credentials
├── data/                       # Runtime persistence database (excluded from version control)
│   └── mosquitto.db
└── log/                        # Broker log files (excluded from version control)
    └── mosquitto.log
```

Note: The data and log directories are created automatically at runtime and should not be committed to version control.

## Configuration Overview

The mosquitto.conf file defines the broker configuration with the following key settings:

- Port 1883: Standard MQTT protocol for ESP32 device connections
- Port 8083: WebSocket protocol for web dashboard connections
- Authentication: Username and password required for all connections
- Persistence: Message queue state saved to disk for reliability
- Logging: Combined console and file output for monitoring

## Installation and Setup

## Installation and Setup

### Prerequisites

- Docker Engine version 20.10 or later
- Docker Compose version 1.29 or later (optional)
- Basic familiarity with MQTT protocol
- Network connectivity for ESP32 and web clients

### Step 1: Create Directory Structure

Create the required directories for authentication, persistence, and logging:

```bash
mkdir -p broker/config/auth broker/data broker/log
```

### Step 2: Generate User Credentials

Create a user account with bcrypt password hashing. Replace 'DataLogger' with your desired username:

```bash
docker run --rm -v "$PWD/broker/config/auth:/work" eclipse-mosquitto:2 \
  mosquitto_passwd -c /work/passwd.txt DataLogger
```

You will be prompted to enter and confirm a password. The -c flag creates a new password file.

To add additional users without overwriting the existing file, omit the -c flag:

```bash
docker run --rm -v "$PWD/broker/config/auth:/work" eclipse-mosquitto:2 \
  mosquitto_passwd /work/passwd.txt additional_user
```

### Step 3: Start the Broker

#### Option A: Docker Run Command

Start the broker as a Docker container with volume mounts for configuration and data:

```bash
docker run -d --name mqtt-broker \
  -p 1883:1883 \
  -p 8083:8083 \
  -v "$PWD/broker/mosquitto.conf:/mosquitto/config/mosquitto.conf" \
  -v "$PWD/broker/config/auth:/mosquitto/config/auth" \
  -v "$PWD/broker/data:/mosquitto/data" \
  -v "$PWD/broker/log:/mosquitto/log" \
  eclipse-mosquitto:2
```

#### Option B: Docker Compose

Create a docker-compose.yml file in the project root:

```yaml
version: '3.8'
services:
  mosquitto:
    image: eclipse-mosquitto:2
    container_name: mqtt-broker
    ports:
      - "1883:1883"
      - "8083:8083"
    volumes:
      - ./broker/mosquitto.conf:/mosquitto/config/mosquitto.conf
      - ./broker/config/auth:/mosquitto/config/auth
      - ./broker/data:/mosquitto/data
      - ./broker/log:/mosquitto/log
    restart: unless-stopped
```

Start the broker:

```bash
docker-compose up -d
```

## Verification and Testing

### Verify Broker Status

Check that the Docker container is running:

```bash
docker ps | grep mqtt-broker
```

Expected output should show the container running with ports 1883 and 8083 exposed.

### Test MQTT Protocol Connection

Open two terminal windows for publish/subscribe testing.

Terminal 1 - Subscribe to test topic:
```bash
mosquitto_sub -h localhost -p 1883 \
  -u DataLogger \
  -P your_password \
  -t "test/topic" \
  -v
```

Terminal 2 - Publish to test topic:
```bash
mosquitto_pub -h localhost -p 1883 \
  -u DataLogger \
  -P your_password \
  -t "test/topic" \
  -m "Hello MQTT"
```

The message should appear in Terminal 1.

### Test WebSocket Connection

The WebSocket endpoint is available at:
```
ws://localhost:8083
```

For remote access, replace localhost with the broker's IP address:
```
ws://192.168.1.100:8083
```

Web clients should connect using MQTT over WebSocket libraries such as MQTT.js.

## Monitoring and Maintenance

### View Broker Logs

Monitor real-time broker activity:

```bash
# View Docker container logs
docker logs -f mqtt-broker

# View file-based logs
tail -f broker/log/mosquitto.log
```

### Check Connection Status

Test broker connectivity:

```bash
mosquitto_pub -h localhost -p 1883 \
  -u DataLogger \
  -P your_password \
  -t "system/ping" \
  -m "test"
```

### Restart Broker

Restart the broker container:

```bash
docker restart mqtt-broker
```

### Clean Restart

Remove the container and clear persistent data:

```bash
docker rm -f mqtt-broker
rm -rf broker/data/* broker/log/*
```

Then start the broker again using the docker run or docker-compose command.

## Configuration Parameters

The mosquitto.conf file contains the following key parameters:

### Network Listeners

```
listener 1883 0.0.0.0
protocol mqtt
```
Binds standard MQTT to port 1883 on all network interfaces.

```
listener 8083 0.0.0.0
protocol websockets
```
Binds MQTT over WebSocket to port 8083 for web browser clients.

### Security Settings

```
allow_anonymous false
password_file /mosquitto/config/auth/passwd.txt
```
Disables anonymous connections and specifies the bcrypt password file location.

### Persistence Configuration

```
persistence true
persistence_location /mosquitto/data
```
Enables message persistence and specifies the database file location.

### Logging Configuration

```
log_type all
log_dest stdout
log_dest file /mosquitto/log/mosquitto.log
```
Enables all log types with output to both console and file.

### Connection Limits

```
max_connections -1
max_inflight_messages 20
max_queued_messages 1000
```
Sets connection and message queue limits. A value of -1 means unlimited connections.

## Troubleshooting

### Connection Refused Error

Symptom: Clients cannot connect to the broker

Possible causes and solutions:
1. Verify broker container is running: `docker ps | grep mqtt-broker`
2. Check firewall rules allow ports 1883 and 8083
3. Verify broker is listening on all interfaces (0.0.0.0)
4. Check Docker port mapping with `docker port mqtt-broker`

### Authentication Failed Error

Symptom: Clients receive authentication errors

Possible causes and solutions:
1. Verify username and password are correct
2. Check passwd.txt file exists and is mounted correctly
3. Confirm bcrypt hashing was used (not plaintext passwords)
4. Review broker logs for authentication failure messages

### WebSocket Connection Failed

Symptom: Web dashboard cannot connect via WebSocket

Possible causes and solutions:
1. Confirm port 8083 is exposed and not blocked by firewall
2. Verify WebSocket protocol is enabled in mosquitto.conf
3. Check browser console for CORS or connection errors
4. Test with mosquitto_sub using --ws-url option if available

### Persistence Not Working

Symptom: Messages are lost after broker restart

Possible causes and solutions:
1. Verify persistence is enabled in mosquitto.conf
2. Check data directory is writable by the mosquitto user
3. Confirm volume mount for data directory is correct
4. Review broker logs for persistence-related errors

### Missing Log Files

Symptom: Log files are not created

Possible causes and solutions:
1. Verify log directory volume mount is correct
2. Check log directory permissions
3. Confirm log_dest file directive in mosquitto.conf
4. Review Docker container logs for error messages

## Security Considerations

### Password Management

- Use strong passwords with mixed case, numbers, and special characters
- Rotate passwords periodically according to security policy
- Store passwd.txt securely and limit file permissions
- Never commit passwd.txt to public version control

### Network Security

- Use TLS/SSL certificates for encrypted connections in production
- Implement firewall rules to restrict broker access
- Consider using VPN for remote broker access
- Enable topic-based access control lists (ACLs) for fine-grained permissions

### Production Deployment

For production environments, enhance security with:

1. TLS/SSL encryption:
```
listener 8883 0.0.0.0
protocol mqtt
cafile /mosquitto/config/certs/ca.crt
certfile /mosquitto/config/certs/server.crt
keyfile /mosquitto/config/certs/server.key
```

2. Topic-based ACL:
```
acl_file /mosquitto/config/acl.conf
```

3. Connection rate limiting
4. Log analysis and monitoring
5. Regular security updates

## Backup and Recovery

### Backup Persistent Data

Create dated backups of the persistence database:

```bash
cp broker/data/mosquitto.db backups/mosquitto_$(date +%Y%m%d_%H%M%S).db
```

### Backup User Credentials

Create dated backups of the password file:

```bash
cp broker/config/auth/passwd.txt backups/passwd_$(date +%Y%m%d_%H%M%S).txt
```

### Automated Backup Script

Create a backup script for periodic execution:

```bash
#!/bin/bash
BACKUP_DIR="backups/$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"
cp broker/data/mosquitto.db "$BACKUP_DIR/"
cp broker/config/auth/passwd.txt "$BACKUP_DIR/"
echo "Backup completed: $BACKUP_DIR"
```

### Restore from Backup

Stop the broker and restore files:

```bash
docker stop mqtt-broker
cp backups/20241023_120000/mosquitto.db broker/data/
cp backups/20241023_120000/passwd.txt broker/config/auth/
docker start mqtt-broker
```

## Integration Examples

### ESP32 Client Connection

Example C code for ESP32 firmware:

```c
#include <WiFi.h>
#include <PubSubClient.h>

const char* mqtt_server = "192.168.1.100";
const int mqtt_port = 1883;
const char* mqtt_user = "DataLogger";
const char* mqtt_password = "your_password";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  client.setServer(mqtt_server, mqtt_port);
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      client.subscribe("datalogger/esp32/command");
    }
  }
}
```

### Web Dashboard Connection

Example JavaScript code for web applications:

```javascript
const client = mqtt.connect('ws://192.168.1.100:8083', {
  clientId: 'WebClient_' + Math.random().toString(16).substr(2, 8),
  username: 'DataLogger',
  password: 'your_password',
  clean: true,
  connectTimeout: 4000,
  reconnectPeriod: 2000
});

client.on('connect', function() {
  console.log('MQTT Connected');
  client.subscribe('datalogger/stm32/#');
});

client.on('message', function(topic, payload) {
  console.log('Received:', topic, payload.toString());
});
```

## Performance Tuning

### High-Traffic Scenarios

For systems with many clients or high message rates:

1. Increase message queue limits:
```
max_queued_messages 5000
max_inflight_messages 50
```

2. Enable message compression in client libraries

3. Monitor memory usage and adjust queue sizes accordingly

### Resource Monitoring

Monitor broker resource usage:

```bash
# CPU and memory usage
docker stats mqtt-broker

# Connection count
docker exec mqtt-broker mosquitto_sub -t '$SYS/broker/clients/connected' -C 1
```

## License

This MQTT broker configuration is part of the DATALOGGER project, licensed under the MIT License. See the project root LICENSE.md file for details.

## Additional Resources

- Eclipse Mosquitto Documentation: https://mosquitto.org/documentation/
- MQTT Protocol Specification: https://mqtt.org/
- Docker Documentation: https://docs.docker.com/


## 🔧 Configuration Features

The broker is configured with the following capabilities:

| Feature | Configuration | Description |
|---------|---------------|-------------|
| **MQTT Protocol** | Port 1883 | Standard MQTT for ESP32 and IoT devices |
| **WebSockets** | Port 8083 | Web dashboard connectivity |
| **Authentication** | Required | No anonymous connections allowed |
| **Persistence** | Enabled | Messages survive broker restarts |
| **Logging** | Dual output | Console + file logging |
| **Connection Limits** | Unlimited | Configurable message queues |

## 🧪 Testing the Setup

### Terminal Testing
**Subscribe to messages** (Terminal 1):
```bash
mosquitto_sub -h localhost -p 1883 -u DataLogger -P your_password -t "sensors/temperature" -v
```

**Publish test message** (Terminal 2):
```bash
mosquitto_pub -h localhost -p 1883 -u DataLogger -P your_password -t "sensors/temperature" -m "23.5"
```

### Web Client Connection
Connect your web dashboard to WebSockets endpoint:
```
ws://localhost:8083
```
or for remote access:
```
ws://your-server-ip:8083
```

## 📊 Monitoring & Logs

### View Live Logs
```bash
# Container logs
docker logs -f mqtt-broker

# File logs
tail -f broker/log/mosquitto.log
```

### Check Broker Status
```bash
# Container status
docker ps | grep mqtt-broker

# Test connectivity
mosquitto_pub -h localhost -p 1883 -u DataLogger -P your_password -t "test/ping" -m "alive"
```

## 🔒 Security Features

- **No Anonymous Access**: All connections require valid credentials
- **Bcrypt Password Hashing**: Secure password storage
- **Connection Limits**: Configurable to prevent DoS attacks
- **Message Queue Limits**: Prevents memory exhaustion

## 🛠️ Troubleshooting

| Issue | Possible Cause | Solution |
|-------|----------------|----------|
| **Connection Refused** | Broker not running | Check `docker ps` and restart if needed |
| **Auth Failed** | Invalid credentials | Verify username/password in `passwd.txt` |
| **WebSocket Connection Failed** | Port 8083 blocked | Check firewall settings |
| **No Persistence** | Volume mount issue | Verify `data/` directory permissions |
| **Missing Logs** | Log volume not mounted | Check `log/` directory mount |

### Common Commands
```bash
# Restart broker
docker restart mqtt-broker

# Clean restart (removes persistence)
docker rm -f mqtt-broker && rm -rf broker/data/* broker/log/*

# View configuration
docker exec mqtt-broker cat /mosquitto/config/mosquitto.conf
```

## 📁 Runtime Data

- **Persistence Database**: `broker/data/mosquitto.db`
- **Authentication File**: `broker/config/auth/passwd.txt`
- **Log Files**: `broker/log/mosquitto.log`
- **Configuration**: `broker/mosquitto.conf`

## 🚀 Production Considerations

### Performance Tuning
- Adjust `max_inflight_messages` based on client load
- Monitor `max_queued_messages` for memory usage
- Enable compression for high-traffic scenarios

### Security Hardening
- Use TLS/SSL certificates for encrypted connections
- Implement topic-based access control (ACL)
- Regular password rotation policy
- Network-level security (VPN, firewall rules)

### Backup Strategy
```bash
# Backup persistence data
cp broker/data/mosquitto.db backups/mosquitto_$(date +%Y%m%d).db

# Backup user credentials
cp broker/config/auth/passwd.txt backups/passwd_$(date +%Y%m%d).txt
```

## 📚 Integration Examples

### ESP32 Connection
```cpp
#include <WiFi.h>
#include <PubSubClient.h>

const char* mqtt_server = "your-server-ip";
const int mqtt_port = 1883;
const char* mqtt_user = "DataLogger";
const char* mqtt_password = "your_password";
```

### Web Dashboard Connection
```javascript
const client = mqtt.connect('ws://your-server-ip:8083', {
  username: 'DataLogger',
  password: 'your_password'
});
```

## License

MIT License - see project root for details.