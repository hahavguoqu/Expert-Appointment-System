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

-   **Framework**: Qt 5
-   **Language**: C++11
-   **Builder**: QMake
-   **Data Storage**: JSON

## 🚀 Getting Started

### Prerequisites

-   A computer with Qt 5 development libraries installed.
-   A C++ compiler (e.g., MinGW for Windows, GCC for Linux, Clang for macOS).

### Installation & Running

1.  **Clone the repository**:
    ```bash
    git clone https://github.com/YourUsername/YourProjectName.git
    cd YourProjectName
    ```

2.  **Open the Project**:
    -   Open the `.pro` file in Qt Creator.
    -   Alternatively, use `qmake` and `make` from the command line:
        ```bash
        qmake
        make
        ```

3.  **Run the Application**:
    -   In Qt Creator, click the "Run" button.
    -   From the command line, launch the generated executable (e.g., `./HospitalAppointmentSystem` on Linux, `release/HospitalAppointmentSystem.exe` on Windows).

## 👨💻 Default Login Accounts

For testing and demonstration purposes, the following expert accounts are pre-configured in the system. The default password for all accounts is **`123456`**.

| Expert ID | Expert Name | Department | Title         |
| :-------: | :---------- | :--------- | :------------ |
|   1001    | 张三 (Zhang San) | 内科         | 主任医师        |
|   1002    | 李四 (Li Si)    | 外科         | 副主任医师      |
|   1003    | 王五 (Wang Wu)  | 儿科         | 主治医师        |
|   1004    | 赵六 (Zhao Liu) | 眼科         | 专家医师        |

**Administrator Access**:
-   **Password**: `123456`

## 📁 Project Structure

```
.
├── src/                    # Source code files
│   ├── mainwindow.h/cpp    # Main window controller
│   ├── expertManager.h/cpp # Expert data management logic
│   ├── appointmentManager.h/cpp # Appointment management logic
│   ├── adminDialog.h/cpp    # Admin UI and logic
│   ├── expertDialog.h/cpp   # Expert UI and logic
│   ├── patientDialog.h/cpp  # Patient UI and logic
│   ├── aiChatDialog.h/cpp  # AI Assistant UI and logic
│   └── ...                 # Other headers and sources
├── data/                   # JSON data files (optional)
├── resources/              # Application resources (images, stylesheets)
└── HospitalAppointmentSystem.pro # Qt Project file
```

## 📄 License

This project is licensed under a Non-Commercial License.

You are free to use, copy, and modify this project for personal or educational purposes.
Commercial use of any kind is strictly prohibited.


---

**Note**: This is a demo application. For security in a production environment, please change the default passwords and implement a more secure authentication system.
