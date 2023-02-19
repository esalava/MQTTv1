# MQTTv1
This is v1. Project suggested by Escuela Superior Politecnica del Litoral (ESPOL).

# What is MQTT?
MQTT service stands for _Message Queuing Telemetry Transfer_. It's a message service that uses pubsub pattern. Publisher and Subscriber will be connected to a common server called broker.


# Purpose
This is an academic purpose. MQTT is a lightweight and flexible network protocol for IoT developers used to be implemented on heavily constrained device hardware as well high latency/limited bandwith network. Its flexibility makes it possible to support IoT devices scenarios. 

# Executables
* Broker
* Publisher
* Subscriber

# How does it work?
Broker's main task is to forward messages from a publisher. Theses messages are divided in topics in a hierarchy. (Topics are explained later on this document). On the other hand subscribers ONLY recieve messages from topics they are interested on filtered by the broker.

#Topics
Topics are scraping from a linux-kernel device with a `top` command. The topics are the following:

* Tasks: related with the CPU information
  * `tasks/gen`: general information (total task)
  * `tasks/run`: running tasks.
  * `tasks/sleep`: sleeping tasks.

* CPU: information related with the CPU usage. (% format)
  * `cpu/gen`: user CPU usage information. (us)
  * `cpu/sys`: system CPU usage information. (sys)
 
* Memory: information related with the RAM usage (% format)
  * `mem/gen`: RAM usage information.

# How to use it?
1. Use 'make' to compile all the object (.o) files
2. Use `./broker` to execute broker process. Broker will use port 8090 for listening connections (This can be configured in `MQTTconfig.h`, ip config as well).
4. Use `./publisher <node_name>` to create a new publisher node process.
4. Use `./suscriber <node_name> <node_subscribed/topic/subtopic>` to create a new subscriber node process.

## Additional features
* You can connect `N` publisher nodes with `M` subscribers nodes.
* This project uses wildcards to get specific information from publishers.
    * Single Level (+): can be used to replace the topics of the same level. ie: node1/+/gen. You will get this data: node1/tasks/gen, node1/cpu/gen, node1/mem/gen.
    * Multileve (#): can be used to cover topics of different levels. It must be used as the last character after a "/". ie: node1/task/#. You'll get node1/tasks/gen, node1/tasks/ram, node1/tasks/sleep.
* Log information taken from broker is located in `log_broker.txt`
