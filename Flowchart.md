# System Workflows & Flowcharts

This document provides visual flowcharts for boot initialization, button gesture recognition, Tuya HTTPS request authorization, and WiFi state management.

---

## 1. System Boot Flowchart

```mermaid
flowchart TD
    Start([Power On / Reset]) --> InitSerial[Init Serial 115200]
    InitSerial --> InitHW[Init LED GPIO8 & Button GPIO9]
    InitHW --> LoadNVS[Load NVS Config & Read Crash Count]
    LoadNVS --> CheckCrash{Crash Count >= 3?}
    
    CheckCrash -- Yes --> EnterSafeMode[Enter SAFE_MODE AP]
    CheckCrash -- No --> RunDiag[Run Boot Self-Diagnostics]
    
    RunDiag --> InitTuya[Init Tuya HTTPS Client & NTP]
    InitTuya --> ConnectWiFi{Saved WiFi Credentials?}
    
    ConnectWiFi -- Found --> TryConnect[Connect to Saved WiFi]
    ConnectWiFi -- None --> LaunchAP[Launch Captive Portal: RemoteSwitchSetup]
    
    TryConnect --> WiFiSuccess{Connected?}
    WiFiSuccess -- Yes --> StartWeb[Start Web Server & REST API]
    WiFiSuccess -- No --> LaunchAP
    
    LaunchAP --> UserConfig[User Enters WiFi SSID/Pwd via Portal]
    UserConfig --> SaveWiFi[Save WiFi & Reboot] --> Start
    
    StartWeb --> InitFSM[Init FSM STATE_READY]
    InitFSM --> Loop([Enter Non-Blocking Scheduler Loop])
```

---

## 2. Button Multi-Gesture State Machine (GPIO9)

```mermaid
flowchart TD
    ButtonPress([GPIO9 State Change]) --> Debounce[Filter 30ms Software Debounce]
    Debounce --> IsPressed{Pin Pressed?}
    
    IsPressed -- Yes --> StartTimer[Record Press Start Time]
    StartTimer --> CheckHold{Hold Duration?}
    
    CheckHold -- "> 15s" --> TriggerReset[Trigger Factory Reset]
    CheckHold -- "> 8s" --> TriggerWiFiReset[Erase WiFi & Restart AP]
    CheckHold -- "> 2s" --> TriggerForceOn[Trigger Tuya Force ON]
    
    IsPressed -- No --> CalcDuration[Calculate Release Duration]
    CalcDuration --> CheckClicks{Click Count & Gap Window}
    
    CheckClicks -- "1 Click (<500ms)" --> TriggerShortPress[Query Tuya Status & Toggle]
    CheckClicks -- "2 Clicks (<350ms gap)" --> TriggerDouble[Trigger Tuya Force OFF]
```

---

## 3. Tuya Cloud OpenAPI Signature Flowchart

```mermaid
flowchart TD
    Request([API Request Triggered]) --> CheckToken{Is Token Valid & Unexpired?}
    
    CheckToken -- No --> FetchToken[HTTP GET /v1.0/token?grant_type=1]
    FetchToken --> GenTokenSign[Compute SHA256 & HMAC-SHA256 Sign]
    GenTokenSign --> SendTokenHTTP[Send Token Request]
    SendTokenHTTP --> TokenOk{Token Success?}
    
    TokenOk -- No --> RaiseError[Log Auth Error & Set LED Rapid Blink]
    TokenOk -- Yes --> CacheToken[Cache Token in RAM with 300s Safety Margin]
    
    CacheToken --> PrepReq
    CheckToken -- Yes --> PrepReq[Prepare Endpoint URL & Body Payload]
    
    PrepReq --> CalcBodyHash[Hash Body payload using SHA256]
    CalcBodyHash --> BuildStringSign[Construct Canonical StringToSign]
    BuildStringSign --> GenReqHMAC[Generate HMAC-SHA256 Sign using Secret]
    GenReqHMAC --> ExecHTTP[Execute HTTPS Request]
    ExecHTTP --> EvaluateResp{HTTP Status 200 & success=true?}
    
    EvaluateResp -- Yes --> Success[Update Local State & Trigger LED Long Blink]
    EvaluateResp -- No --> RetryCheck[Check Retry Count <= 3]
```
