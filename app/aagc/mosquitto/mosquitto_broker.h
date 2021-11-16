#ifndef MOSQUITTO_BROKER_H
#define MOSQUITTO_BROKER_H

int start_mosquitto_client(char *prog_name);
void stop_mosquitto_client();

int my_publish(const char *topic, int payloadlen, void *payload);



#endif// MOSQUITTO_BROKER_H