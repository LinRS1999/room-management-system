from PyQt5.QtWidgets import *
import socket

class refund(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('客房管理系统')
        self.initUI()

    def initUI(self):
        self.vbox = QVBoxLayout()
        self.lineEdit = QLineEdit()
        self.form2 = QFormLayout()
        self.form2.addRow('请输入您的姓名：', self.lineEdit)
        self.widget = QWidget()
        self.widget.setLayout(self.form2)
        self.vbox.addWidget(self.widget)
        # 设置IP和端口号
        self.h = '172.20.10.2'
        self.p = 5000
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((self.h, self.p))
        self.button = QPushButton('确认退房')
        self.button.clicked.connect(self.delete)
        self.vbox.addWidget(self.button)
        self.setLayout(self.vbox)
    # 删除
    def delete(self):
        reply = QMessageBox.question(self, '确认退房', '你确认要退房吗？', QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
        if reply == QMessageBox.Yes:
            name = self.lineEdit.text()
            flag = self.deletesql(name)
            if flag > 0:
                QMessageBox.about(self, ' ', '退房成功！')
            else:
                QMessageBox.about(self, ' ', '退房失败！')

    def deletesql(self, name):
        temp = str(name)
        self.s.send(str.encode(temp))
        res = self.s.recv(512)
        print(res)
        if 'error' not in str(res):
            return 1
        else:
            return 0