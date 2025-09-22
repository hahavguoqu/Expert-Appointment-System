Read this in: [English](README.md) | [简体中文](README.zh.md)

# 医院专家预约系统

一个基于 Qt 开发的功能完善、用户友好的桌面应用程序，用于管理医疗专家预约。系统支持三类不同角色（患者、医疗专家和管理员），以实现便捷的挂号、智能的排班管理和全面的数据监督。

## ✨ 功能特点

-   **👥 基于角色的访问**：为患者、专家和管理员提供定制化的界面。  
-   **📅 可视化与灵活排班**：专家可以设置每周的常规出诊时间，以及特殊日期的自定义时段和接待上限。  
-   **🆔 智能输入校验**：自动解析中国身份证号以提取性别和年龄，并实时验证手机号。  
-   **🤖 AI 助手**：集成 AI API，支持自然语言提问以查询排班和可用性。  
-   **💾 数据持久化**：所有数据均通过 JSON 文件保存和加载，支持导入/导出以便备份和管理。  
-   **🎨 现代化图形界面**：基于 Qt Widgets 开发，提供简洁直观的用户体验。  

## 🛠️ 技术栈

-   **框架**：Qt 5.12.10  
-   **语言**：C++11  
-   **构建工具**：QMake  
-   **编译器**：MinGW_64  
-   **数据存储**：JSON  

## 👨💻 默认登录账号

为测试和演示目的，系统中预先配置了以下专家账号。所有账号的默认密码均为 **`123456`**。  

| 专家ID | 专家姓名 | 科室 | 职称       |
| :----: | :------: | :--: | :--------- |
|  1001  |   张三   | 内科 | 主任医师   |
|  1002  |   李四   | 外科 | 副主任医师 |
|  1003  |   王五   | 儿科 | 主治医师   |
|  1004  |   赵六   | 眼科 | 专家医师   |

**管理员账号**：  

-   **密码**：`123456`  

## 📁 项目结构

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
│   └─── hospital_logo.png             
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

## 📄 许可协议

本项目采用 **非商业许可证 (Non-Commercial License)**。  

您可以自由地使用、复制和修改本项目用于个人或教育目的。  
**严禁任何形式的商业用途。**