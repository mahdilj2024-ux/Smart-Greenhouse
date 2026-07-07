# 🌱 EdgeSense Platform

### Industrial Edge IoT Monitoring Platform | Smart Plant Monitoring Demo

> A production-oriented Edge IoT platform demonstrating how embedded hardware, edge computing, Docker, and mechanical design can be integrated into a complete monitoring solution.

<img width="2700" height="2160" alt="Untitled Project" src="https://github.com/user-attachments/assets/9ef67a04-49c2-4fd2-8ce8-bd2d0078b03f" />



## 📌 Overview

EdgeSense Platform is an end-to-end IoT system designed to monitor environmental conditions, automate irrigation, and visualize real-time data through an edge computing architecture.

Although the current implementation is demonstrated on a single smart plant, the software architecture is intentionally designed to support multiple sensor nodes without major modifications.

This project combines:

* Embedded Systems
* Edge Computing
* Dockerized Services
* MQTT Communication
* Data Monitoring
* Mechanical Product Design

---

# 🚀 Project Objectives

* Build a scalable Edge IoT architecture
* Demonstrate complete product development workflow
* Design a modular hardware platform
* Implement reliable MQTT communication
* Deploy services using Docker
* Showcase mechanical enclosure design with SolidWorks

---

# 🏗️ System Architecture

```
ESP32 Sensor Node
        │
        ▼
 MQTT (Mosquitto)
        │
        ▼
 Raspberry Pi Gateway
        │
        ▼
 Php Data Service
        │
        ▼
     MySQL Database
        │
        ▼
   Monitoring Dashboard
```

---

# 🔧 Hardware

## Edge Node

* ESP32
* DHT22 Temperature & Humidity Sensor
* Soil Moisture Sensor
* Relay Module
* Water Pump

## Edge Gateway

* Raspberry Pi
* Mosquitto MQTT Broker
* Docker
* Php

---

# 💻 Software Stack

* ESP-IDF
* MQTT
* Docker
* Mosquitto
* MySQL
* Git
* GitHub

---

# 📦 Docker Services

The gateway services are deployed using Docker.

Example:

* MQTT Broker
* Database
* Dashboard

Deployment:

```bash
docker compose up -d
```

---

# ⚙️ Features

* Real-time environmental monitoring
* Automated irrigation
* MQTT publish/subscribe communication
* Dockerized deployment
* Raspberry Pi edge gateway
* Sensor data logging
* Modular architecture
* Mechanical enclosure designed in SolidWorks

---

# 🧩 Mechanical Design

The enclosure was designed in SolidWorks to provide:

* Compact assembly
* Component protection
* Easy maintenance
* Organized cable routing
* Prototype-ready design

CAD files are available inside the **/cad** directory.

---

# 📂 Repository Structure

```
/
├── firmware/
├── docker/
├── docs/
├── cad/
├── wiring/
├── images/
├── videos/
└── README.md
```

---

# 📈 Current Demonstration

Current implementation includes:

* Single physical ESP32 node
* Raspberry Pi edge gateway
* MQTT communication
* Local database
* Docker deployment
* Mechanical enclosure

The architecture has been designed to support multiple sensor nodes in future deployments.

---

# 🎯 Future Improvements

* OTA firmware updates
* REST API
* Multi-user authentication
* Grafana dashboard
* InfluxDB integration
* Telegram notifications
* Device provisioning
* Predictive analytics

---

# 📚 Documentation

Detailed documentation is available inside the **/docs** folder.

* Installation Guide
* Hardware Documentation
* MQTT Topics
* Docker Setup
* Wiring Guide

---

# 🛠️ Engineering Highlights

* Embedded firmware development
* Edge computing architecture
* MQTT-based communication
* Dockerized deployment
* Mechanical enclosure design
* Modular system architecture
* End-to-end IoT integration

---

# 👨‍💻 Author

Designed and developed by **Mahdi Javahery**

Specialization:

* Embedded Systems
* IoT
* Mechanical Design
* Product Development
* Industrial Automation

---

# ⭐ If you found this project interesting, consider giving it a star.
