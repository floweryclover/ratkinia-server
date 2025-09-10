# Ratkinia Server
* MMORPG 서버 프로젝트
* ECS를 이용한 게임 로직 + IOCP 기반 네트워크 로직
* OpenSSL, libpqxx, Protocol Buffers 사용

## 핵심 구현
* 25년 8월까지 서버 코어 구현 완료
    * 네트워크 서버 ↔ 메인 서버 메시지 송수신 로직
    * OpenSSL 기반 TLS 통신
* 25년 9월~ 게임 로직 구현 중
    * ECS 기반 메인 로직
    * 현재 로그인, 캐릭터 선택 후 월드 접속 구현

## 전체 클래스 다이어그램
![클래스다이어그램](https://github.com/user-attachments/assets/9d32ad0b-0e6b-4c2f-a1fe-9d122fd021b9)

## 네트워크 서버 ↔ 메인 서버 시퀀스 다이어그램
![시퀀스다이어그램](https://github.com/user-attachments/assets/f07959a9-5d73-4ca1-9112-09d2f05e40f2)