# 📦 스마트렉 하드웨어 컨트롤러 (Arduino / ESP32)

> 스마트 물류 선반(스마트렉)의 ESP32 펌웨어
>
> WiFi·MQTT로 서버와 연동하고, BLE로 작업자의 근접을 감지하며, 로드셀(무게 센서)로 입·출고 작업의 정확성을 검증하고 LED로 작업자를 안내하는 IoT 하드웨어 컨트롤러입니다.

---

## 📌 프로젝트 소개

스마트렉은 작업자가 선반에서 물품을 넣고(입고) 빼는(출고) 과정을, **무게 측정을 통해 실시간으로 검증**하는 물류 보조 시스템입니다. 이 레포지토리는 그 선반에 부착되는 ESP32 디바이스의 펌웨어로, 다음 역할을 담당합니다.

- **서버 연동** — MQTT(TLS)로 작업 지시를 수신하고, 검증 결과를 다시 서버로 전송
- **작업자 인식** — BLE 스캔으로 작업자(휴대폰)의 근접 여부를 신호 세기(RSSI)로 판단
- **작업 안내** — 작업 유형(입고/출고)에 맞는 LED를 점등해 작업자에게 위치를 안내
- **무게 검증** — 로드셀(HX711)로 측정한 무게 변화가 기대값과 일치하는지 품목 단위로 검증

---

## 🔁 동작 개요

```
        ┌──────────────┐   MQTT(assign)    ┌─────────────────────────────┐
        │    서버       │ ────────────────► │           ESP32              │
        │  (MQTT Broker)│                   │                              │
        │              │ ◄──────────────── │  WiFi 연결 → MQTT(TLS) 연결   │
        └──────────────┘   MQTT(esp/ack)   │      ↓                       │
                                           │  BLE 스캔 (작업자 근접 감지)  │
                                           │      ↓ (RSSI > -45)          │
                                           │  LED 점등 (입고/출고 안내)    │
                                           │      ↓                       │
                                           │  로드셀 무게 측정 & 검증      │
                                           │      ↓                       │
                                           │  결과(good/bad) MQTT 전송     │
                                           └─────────────────────────────┘
```

메인 루프(`loop`)는 약 2초 주기로 다음을 반복합니다.

1. `connectWiFi()` — WiFi 연결 유지(끊기면 재연결)
2. `connectMQTT()` — MQTT 연결이 끊겼으면 재연결, 연결 시 작업 지시 토픽 구독
3. `mqttClient.loop()` — MQTT 메시지 수신 처리
4. `processTasks()` — BLE 스캔으로 작업자 근접을 확인하고, 가까우면 무게 검증 수행

---

## ✨ 주요 기능

### 1. WiFi 연결 (`wifiConnect`)
- 연결이 끊겨 있을 때만 재연결을 시도하며, 최대 20회까지 0.5초 간격으로 대기합니다.

### 2. MQTT (TLS) 통신 (`mqttControl`)
- `WiFiClientSecure` + `PubSubClient` 기반의 보안 MQTT 연결
- **구독**: `server/z01-s01/assign` — 서버로부터 작업 지시(작업 유형, 작업자, 품목 목록) 수신
- **발행**: `esp/ack` — 각 품목의 검증 결과(`good`/`bad`) 전송
- 수신한 JSON에서 품목 정보(상품ID·무게·수량)를 파싱해 내부 `productMap`에 저장

### 3. BLE 근접 감지 (`bleScan`)
- 주변 BLE 기기를 스캔하여 이름에 `Quantum3`가 포함된 기기(작업자 단말)를 탐색
- 신호 세기(RSSI)가 가장 강한 값을 선택하고, **RSSI > -45**(매우 가까움)일 때 하드웨어 동작을 트리거

### 4. LED 작업 안내 (`ledControl`)
- 수신한 작업 유형(`work_type`)에 따라 해당 LED 점등
  - **`OB`(출고)** → 빨간색 LED
  - **`IB`(입고)** → 초록색 LED
- 작업자 ID(`worker_id`) 검증을 통과한 경우에만 동작

### 5. 로드셀 무게 검증 (`loadcell`)
프로젝트의 핵심 로직입니다. HX711 로드셀로 측정한 무게 변화를, 서버가 보낸 기대값(품목 무게 × 수량)과 비교해 품목 단위로 검증합니다.

- **입고(IB)**: 작업자가 품목을 올릴 때마다 아래 세 조건을 모두 만족하는지 확인
  1. 측정된 총 무게 == 기대 누적 총합
  2. 무게 증가량 == 해당 품목의 기대 무게
  3. 개당 증가 무게 == 기대 개당 무게
- **출고(OB)**: 먼저 선반에 전체 물품이 올바르게 올라가 있는지 초기 상태를 점검한 뒤, 품목을 뺄 때마다 위와 대응하는 세 조건(총합·감소량·개당 무게)을 검증
- 각 품목 검증이 끝나면 결과를 `esp/ack` 토픽으로 전송하고, 다음 품목으로 진행. 모든 품목 완료 시 LED를 끔

### 6. JSON 파싱 유틸 (`utils`)
- `ArduinoJson` 기반의 공통 파싱 함수로, MQTT 페이로드 해석을 단일 함수로 통일

---

## 🔌 하드웨어 구성

| 구성 요소 | 설명 | 핀 |
|-----------|------|----|
| ESP32 | 메인 컨트롤러 (WiFi · BLE 내장) | — |
| HX711 + 로드셀 | 무게 측정 | DOUT: 4, SCK: 5 |
| LED (빨강) | 출고(OB) 안내 | 26 |
| LED (초록) | 입고(IB) 안내 | 27 |
| LED (노랑) | 상태 표시 | 14 |

> 로드셀 보정 계수(`calibration_factor`)는 코드에 `430000`으로 설정되어 있으며, 사용하는 로드셀에 맞게 조정이 필요합니다.

---

## 📡 MQTT 인터페이스

### 수신 (구독: `server/z01-s01/assign`)
작업 지시 메시지 예시:
```json
{
  "work_type": "IB",
  "worker_id": "2011",
  "products": [
    { "product_id": "P001", "weight": 12.5, "quantity": 3 }
  ]
}
```

### 송신 (발행: `esp/ack`)
검증 결과 메시지 예시:
```json
{
  "product_id": "P001",
  "worker_id": "2011",
  "location_id": "z01-s01",
  "code": "good"
}
```

> `work_type` 은 `IB`(입고) / `OB`(출고), `code` 는 `good`(검증 성공) / `bad`(검증 실패)를 의미합니다.

---

## 🛠 기술 스택 / 라이브러리

| 구분 | 내용 |
|------|------|
| 플랫폼 | ESP32 (Arduino Framework) |
| 언어 | C / C++ |
| 통신 | WiFi, MQTT over TLS |
| 사용 라이브러리 | `WiFi`, `WiFiClientSecure`, `PubSubClient`(MQTT), `BLEDevice`/`BLEScan`(ESP32 BLE), `HX711`(로드셀), `ArduinoJson` |

---

## 📂 프로젝트 구조

```
arduino-hw-controller
├── totalCodeForHardware.ino   # 메인 스케치 (setup / loop)
├── wifiConnect.cpp / .h       # WiFi 연결
├── mqttControl.cpp / .h       # MQTT(TLS) 연결·구독·발행, 작업 지시 파싱
├── bleScan.cpp / .h           # BLE 스캔, 작업자 근접(RSSI) 감지
├── ledControl.cpp / .h        # 작업 유형별 LED 제어
├── loadcell.cpp / loalCell.h  # 로드셀 무게 측정 및 입·출고 검증
├── utils.cpp / .h             # JSON 파싱 공통 유틸
└── secret.h                   # WiFi / MQTT 인증 정보 (직접 작성)
```

---

## ⚙️ 설정 및 빌드

### 1. 인증 정보 설정 (`secret.h`)
`secret.h` 에 아래 값을 채워 넣습니다. **민감 정보이므로 공개 저장소에 실제 값을 커밋하지 않도록 주의하세요.**
```cpp
#define WIFI_SSID      "..."
#define WIFI_PASSWORD  "..."
#define MQTT_BROKER    "..."
#define MQTT_PORT      8883
#define MQTT_USER      "..."
#define MQTT_PASSWORD  "..."
```

### 2. 라이브러리 설치
Arduino IDE의 라이브러리 매니저에서 다음을 설치합니다.
- `PubSubClient`
- `HX711`
- `ArduinoJson`
- ESP32 보드 패키지(WiFi · BLE 포함)

### 3. 업로드
- 보드: ESP32 계열 선택
- 시리얼 모니터 통신 속도: **115200 baud**
- 보드 연결 후 스케치 업로드

---

## 👥 팀

- **KSEB-Rescue-Pang** — 스마트렉 프로젝트

---

## 📎 Repository

- https://github.com/KSEB-Rescue-Pang/arduino-hw-controller
