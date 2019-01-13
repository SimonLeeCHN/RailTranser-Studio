# SimpleActionPlayer
标签：舞台系统 Cmd-Markdown

*cmd markdown编写*



## 版本说明

**V3.0.4**
实现所有功能

**V3.0.5**
添加了唤起外部CASF-Creator的功能

**V3.0.6**
调试优化，在更新GraphicCarrier位置后强制调用scene的advance（未测试，看能否消除列表已刷新而轨道图上未刷新的情况-测试发现好像没用）
注册文件关联添加了确认信息框
调试输出窗口增加了时间戳以及右键确认清除框

**V3.0.7**
通信方式修改，在上位机打开端口后定时心跳包检测载体车是否就绪，检测到载体车全部就绪后，关闭心跳包，防止半双工堵塞。载体车运动过程中会主动发送心跳包

Relocate按钮之和串口是否打开逻辑相关，否则用户在紧急停车之后，重新打开串口，则会因为载体车一直发送紧急停止的心跳包而无法进入准备就绪的状态

修正：mainwindow.cpp中的m_iCarrierNum是直接从工程文件的数字读取，但是工程文件可能有错误导致其内标识的数字和实际的载体车配置行匹配不上

修正：用户关串口后，会将心跳包记录表清除，避免再次打开串口后会直接判断为都就绪

**DL-Zigbee_V3.0.7**
为DL zigbee特定修改的基于V3.0.7的版本

**DL-Zigbee_V3.1.0版本**
增加carriermanager进行carrier管理
包含polling查询与纠错，极大的提高了纠错性

**V3.2.0版本**
主界面移除了端口配置，端口配置在新设置窗体中完成
加入了toolbox窗体
命名不再加入DL-Zigbee
主界面加入了dockwidget

**V3.2.1版本**
inmotionloop每次调用先关闭超时定时器
actionplayer中需要自调用，不使用this->doNextStep这种方法，避免递归深度过深使用单定时触发器，定时极短时间后触发doNextStep
CMG逻辑修改，允许混合指令编组（如MOV DEY MOV)

