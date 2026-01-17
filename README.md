# AVaOut: Avatar Out

<img width="756" height="535" alt="image" src="https://github.com/user-attachments/assets/d4943760-74d9-4432-a890-c56edc65c6e0" />

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.6.1-0E1128?style=for-the-badge&logo=unrealengine)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6?style=for-the-badge&logo=windows)

## 📖 목차
- [프로젝트 소개](#-프로젝트-소개)
- [주요 기능](#-주요-기능)
- [핵심 기술](#-핵심-기술)
- [기술 스택](#-기술-스택)
- [게임 플레이 영상](#-게임-플레이-영상)
- [팀 구성](#-팀-구성)
- [참고 링크](#-참고-링크)

---

## 🎮 프로젝트 소개
<img width="300" height="300" alt="image" src="https://github.com/user-attachments/assets/78e9b80c-7417-4943-9b81-32294e8c4a48" />

### 게임 정보
- **제목**: AVaOut: Avatar Out
- **장르**: 멀티플레이 협동 서바이벌, 퍼즐
- **플랫폼**: PC (Windows)
- **엔진**: Unreal Engine 5.6.1
- **개발 기간**: 2025.11 ~ 2026.01 (3개월)
- **팀 구성**: 8인 (김피탕 컴퍼니)

### 📖 게임 개요

> 멸망해가는 모행성을 떠나기 위해 테라포밍 후보 행성을 조사하는 아바타 탐사단의 여정이 시작된다.
> 
> 탐사단은 위험천만한 외계 행성을 직접 조사하는 대신 원격으로 '아바타'를 조종하여 임무를 수행한다.
> 
> 플레이어는 연락이 두절된 행성 "엘리시움7"을 조사하는 임무를 받게 되는데, 예상치 못한 사고로 연료를 잃고 불시착하게 된다.
> 
> 임무를 무사히 완수하기 위해서는 **부족한 연료를 수집**하고, 행성 곳곳에 흩어진 **선발대의 탐사 기록을 확보**해야 한다.
> 
> 미지의 행성과 적대적인 생명체로부터 살아남고 무사히 임무를 완수하라!

---

## ✨ 주요 기능

<img width="1920" height="1038" alt="image" src="https://github.com/user-attachments/assets/806a2001-95f7-4524-9f3a-e7fe14b3c437" />

### 🎭 메타휴먼 기반 캐릭터 시스템
- **고품질 메타휴먼 캐릭터** 활용
- **실시간 커스터마이징 시스템**: 의상, 외형 변경
- **Mutable 시스템** 활용한 효율적인 캐릭터 커스터마이징

### 🤝 멀티플레이 협동 시스템
- **최대 4인 협동 플레이** 지원
- **실시간 음성 채팅** 시스템
- **Steam OSS(Online Subsystem) 기반** 멀티플레이 구현

### 🏃 파쿠르 & 이동 시스템
- **GAS(Gameplay Ability System)** 기반 캐릭터 이동
- **Motion Matching** 기술을 활용한 자연스러운 애니메이션
- **파쿠르 시스템**: 벽 오르기, 점프, 스프린트, 웅크리기
</br>

<img width="1479" height="830" alt="image" src="https://github.com/user-attachments/assets/fc67260b-8444-4594-a45c-bec9b8087456" />


### 🧠 AI 시스템
- **Behavior Tree** 기반 AI
- **EQS(Environment Query System)** 활용
- **늑대인간(Werewolf)** 등 다양한 적대 생명체
- **Sight Perception** 기반 플레이어 인지
</br>

<img width="838" height="872" alt="image" src="https://github.com/user-attachments/assets/c9197f00-3723-40e6-9547-f742f6987a69" />

### 🧩 퍼즐 시스템
- **Tag 기반** 확장 가능한 퍼즐 시스템
- **협동 퍼즐**: 팀원 간 협력 필수
- **환경 상호작용** 요소
</br>

<img width="768" height="359" alt="image" src="https://github.com/user-attachments/assets/2cc00cb8-9a09-4a96-be9b-358b7c95a48e" />

### 🎨 비주얼 & UI
- **Lumen & Nanite** 활용한 고품질 그래픽
- **직관적인 UI/UX** 설계
- **게임 설정**: 그래픽 품질, 해상도, 음량 조절
</br>

<img width="436" height="212" alt="image" src="https://github.com/user-attachments/assets/83355030-d409-4ad4-a051-03835cbab877" />

### 📄 아이템 및 인벤토리 시스템
- **다양한 아이템 수집**: 연료, 소모품, 패시브 아이템, 탐사 기록
- **인벤토리 관리**: UI 기반 직관적인 아이템 관리 시스템
- **아이템 상호작용**: 팀원간 아이템 공유 및 협력 플레이 유도

---

## 🔧 핵심 기술

### Gameplay Ability System (GAS)
- 캐릭터 이동(파쿠르, 점프, 스프린트) Ability 구현
- Attribute Set 기반 체력, 스태미나 관리
- Gameplay Tag 기반 상태 관리
- Local Prediction을 활용한 멀티플레이 최적화

### Motion Matching
- Experimental 기술을 프로덕션 환경에 적용
- 메타휴먼 캐릭터의 자연스러운 애니메이션 전환
- 모션 워핑(Motion Warping)과 통합

### 멀티플레이 네트워킹
- Steam 세션 생성 및 참가 시스템
- Listen Server 기반 멀티플레이
- 실시간 음성 채팅

### Mutable 커스터마이징 시스템
- 실시간 캐릭터 외형 변경
- 메모리 효율적인 커스터마이징 관리
- 레벨 전환 시에도 커스터마이징 상태 유지

### 음성 채팅 (IOnlineVoice Steam)
- Online Subsystem Steam, IOnlineVoice 기반 음성 채팅 
- Mute/Unmute 기능
- 레벨 전환 시에도 안정적인 음성 채팅 유지

### AI & EQS
- Behavior Tree 기반 복잡한 AI 행동 패턴
- EQS를 활용한 포위 공격, 순찰 등
- Sound Perception으로 플레이어 위치 인지

### 카오스 디스트럭션
- 물리 기반 파괴 시스템
- 퍼즐 요소와 통합
- 멀티플레이 환경에서의 일관성 확보

### 아이템 & 인벤토리
- CSV 파일을 통한 DataTable 관리
- Item ID를 통한 인벤토리 관리
- GAS를 활용한 캐릭터의 능력치 변경
  
### 레벨 최적화
- 텍스처 스트리밍 최적화
- 비디오 메모리 관리

---

## 🛠 기술 스택

### 엔진 & 언어
- Unreal Engine 5.6.1
- C++
- Blueprint

### 주요 시스템
- Gameplay Ability System (GAS)
- Motion Matching
- Mutable
- Chaos Destruction
- Enhanced Input System
- Online Subsystem Steam
- IOnlineVoice

### 네트워킹
- Listen Server
- Replication
- RPC (Remote Procedure Call)

### AI
- Behavior Tree
- EQS (Environment Query System)
- AI Perception

--- 

## 🎥 게임 플레이 영상

### Game Trailer
[Youtube - [Trailer] AVaOut : Avatar Out](https://www.youtube.com/watch?v=bVIziMrU1Mw)

### Gameplay
[Youtube - [Demo] AVaOut : Avatar Out (12min)](https://www.youtube.com/watch?v=vRxYs1qoXhI)

---

## 👥 팀 구성

<img width="715" height="458" alt="image" src="https://github.com/user-attachments/assets/035fe3a2-ac62-415a-af96-260ab060f65a" />

- 자세한 팀 구성 및 담당 기능은 노션 페이지를 참고해주세요.

---

## 🔗 참고 링크
- ​GitHub Repository​: [AVaOut GitHub](https://github.com/NbcampUnreal/3rd_4th-Team4-CH6-Project)
- ​Notion 프로젝트 페이지​: [AVaOut 노션](https://pentagonal-street-989.notion.site/AVaOut-Avatar-Out-2e725419c56b80e18edcee7934e889f5?source=copy_link)

---

## License
- This project is licensed under the MIT License.
- © 2025 김피탕 컴퍼니 - AVaOut: Avatar Out
