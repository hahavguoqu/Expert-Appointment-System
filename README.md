---

# Hospital Expert Appointment System

A comprehensive and user-friendly desktop application for managing medical expert appointments, built with Qt. This system supports three distinct roles (Patients, Medical Experts, and Administrators) to facilitate seamless booking, intelligent schedule management, and robust data oversight.

## ✨ Features

-   **👥 Role-Based Access**: Tailored interfaces for Patients, Experts, and Administrators.
-   **📅 Visual & Flexible Scheduling**: Experts can set regular weekly availability and special dates with custom time slots and capacity limits.
-   **🆔 Smart Input Validation**: Automatically parses Chinese ID cards to extract gender and age. Validates phone numbers in real-time.
-   **🤖 AI-Powered Assistant**: Integrates with an AI API to answer natural language questions about schedules and availability.
-   **💾 Data Persistence**: All data is saved and loaded from JSON files, supporting easy import/export for backup and administration.
-   **🎨 Modern GUI**: Developed with Qt Widgets, providing a clean and intuitive user experience.

## 🛠️ Tech Stack

-   **Framework**: Qt 5.12.10
-   **Language**: C++11
-   **Builder**: QMake 
-   **Compiler**：MinGW_64
-   **Data Storage**: JSON

## 👨💻 Default Login Accounts

For testing and demonstration purposes, the following expert accounts are pre-configured in the system. The default password for all accounts is **`123456`**.

| Expert ID | Expert Name | Department | Title         |
| :-------: | :---------- | :--------- | :------------ |
|   1001    | 张三 | 内科         | 主任医师        |
|   1002    | 李四    | 外科         | 副主任医师      |
|   1003    | 王五  | 儿科         | 主治医师        |
|   1004    | 赵六 | 眼科         | 专家医师        |

**Administrator Access**:
-   **Password**: `123456`

## 📁 Project Structure

```
HospitalExpertAppointmentSystem/  
│
├── HospitalExpertAppointmentSystem.pro    
├── .gitignore                            
├── README.md                         
│
├── main.cpp                          
├── mainwindow.h                     
├── mainwindow.cpp                      
├── expert.h                           
├── expert.cpp                         
├── appointment.h                    
├── appointment.cpp                   
├── expertManager.h                   
├── expertManager.cpp               
├── appointmentManager.h               
├── appointmentManager.cpp              
├── adminDialog.h                      
├── adminDialog.cpp                   
├── expertDialog.h                     
├── expertDialog.cpp                   
├── patientDialog.h                 
├── patientDialog.cpp                 
├── aiChatDialog.h  
├── aiChatDialog.cpp                  
├── adminDialog.ui
├── expertDialog.ui
├── patientDialog.ui
├── aiChatDialog.ui
├── mainwindow.ui
│
├── images/
│   ├── doctor.png                    
│   └── hospital_logo.png             
│   
│
├── styles/    
│   └── application.qss
│   
│
└── resource/                      
    ├── experts.json                 
    └── appointments.json         

```

## 📄 License

This project is licensed under a Non-Commercial License.

You are free to use, copy, and modify this project for personal or educational purposes.
Commercial use of any kind is strictly prohibited.
