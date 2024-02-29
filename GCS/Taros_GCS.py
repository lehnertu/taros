#!/usr/bin/env python3

# import numpy as np
from struct import *
import random

import PySide6
from PySide6.QtCore import Qt
from PySide6 import QtWidgets
from PySide6.QtSerialPort import *
from PySide6.QtGui import QColor
from PySide6.QtWidgets import ( QApplication, QWidget, QTabWidget,
    QTableWidget,  QTableWidgetItem, QHeaderView,
    QPushButton, QLabel, QGridLayout, QHBoxLayout, QVBoxLayout )


def format_time(t):
    h = t//(1000*60*60)
    ms = t-h*(1000*60*60)
    m = ms//(1000*60)
    ms = ms-m*(1000*60)
    s = ms//1000
    ms = ms-s*1000
    return "%02d:%02d:%02d.%03d" % (h,m,s,ms)

class CommunicationsView(QWidget):
    
    def __init__(self, main_window):
        QWidget.__init__(self)
        self.mv = main_window
        layout = QHBoxLayout()
        layout.setContentsMargins(20,20,20,20)
        self.setLayout(layout)
        # port for listening
        self.port_available = False
        self.port = None
        # receive buffer for the messages
        self.receive_buffer = bytearray(b'')
        # the left side - message table
        self.table = QTableWidget()
        self.table.setRowCount(0)
        self.table.setColumnCount(4)
        self.table.setHorizontalHeaderLabels(["Source", "Time", "Message", "RSI"])
        header = self.table.horizontalHeader()       
        header.setSectionResizeMode(0, QHeaderView.ResizeMode.ResizeToContents)
        header.setSectionResizeMode(1, QHeaderView.ResizeMode.ResizeToContents)
        header.setSectionResizeMode(2, QHeaderView.ResizeMode.Stretch)
        header.setSectionResizeMode(3, QHeaderView.ResizeMode.ResizeToContents)
        layout.addWidget(self.table,75)
        layout.addSpacing(20)
        # the right side - column of buttons
        rlayout = QVBoxLayout()
        layout.addLayout(rlayout,25)
        clear_button = QPushButton("Clear")
        clear_button.clicked.connect(self.clear_list)
        self.status = QLabel("None.")
        refresh_button = QPushButton("Refresh")
        refresh_button.clicked.connect(self.refresh)
        open_button = QPushButton("Open Port")
        rlayout.addWidget(clear_button)
        rlayout.addWidget(self.status)
        rlayout.addWidget(refresh_button)
        rlayout.addWidget(open_button)
        open_button.clicked.connect(self.open_serial_port)

    def refresh(self):
        self.status.setText("searching devices ...")
        try:
            portinfo = QSerialPortInfo()
            avail = portinfo.availablePorts()
            self.port_available = (len(avail)>0)
            if self.port_available:
                self.port = avail[0]
                self.status.setText("using port " + self.port.portName())
            else:
                self.status.setText("no available port found.")
        except Exception as e:
            # print(e)
            pass
            
    def open_serial_port(self):
        if self.port_available:
            try:
                self.serial = QSerialPort()
                self.serial.setPortName(self.port.portName())
                self.serial.setBaudRate(QSerialPort.Baud115200, QSerialPort.AllDirections)
                self.serial.setParity(QSerialPort.NoParity)
                self.serial.setStopBits(QSerialPort.OneStop)
                self.serial.setDataBits(QSerialPort.Data8)
                self.serial.setFlowControl(QSerialPort.NoFlowControl)
                self.serial.open(QSerialPort.ReadWrite)
                self.serial.readyRead.connect(self.on_serial_read)
                text = self.status.text()
                text += "\n"
                text += "port open."
                self.status.setText(text)
                # print(text)
            except Exception as e:
                self.port_available = False
                self.status.setText("error opening port.")
                # print(e)
                
    def on_serial_read(self):
        """
        Called when the application gets data from the connected device.
        """
        msg = bytes(self.serial.readAll())
        self.receive_buffer.extend(msg)
        # line = ""
        # for c in msg:
        #     line += (" %0.2X" % c)
        # print(line)
        
        found = False
        empty = len(self.receive_buffer) < 3
        while not (found or empty):
            msb, lsb, n_bytes = unpack('BBB', self.receive_buffer[:3])
            # if it ihas a message header
            if msb == 204:
                # if it is a system message or a ping response
                if lsb == 129 or lsb == 136:
                    if len(self.receive_buffer) >= n_bytes+3:
                        found = True
                    else:
                        empty = True
            if not (found or empty):
                self.receive_buffer.pop(0)
                empty = len(self.receive_buffer) < 3
                
        if found:
            msg = self.receive_buffer[0:n_bytes+4]
            del self.receive_buffer[0:n_bytes+4]
            msb, lsb, n_bytes = unpack('BBB', msg[:3])
            # if it ihas a message header
            if msb == 204:
                # if it is a system message
                if lsb == 129:
                    sender = msg[3:11].decode(encoding='utf-8')
                    self.next_index = self.table.rowCount()
                    self.table.insertRow(self.next_index)
                    self.table.setRowCount(self.next_index+1)
                    level = msg[11]
                    if level==1: # MSG_LEVEL_FATALERROR
                        col = QColor.fromRgb(200, 0, 0)
                    elif level==3: # MSG_LEVEL_CRITICAL
                        col = QColor.fromRgb(255, 0, 0)
                    elif level==5: # MSG_LEVEL_MILESTONE
                        col = QColor.fromRgb(0, 200, 0)
                    elif level==8: # MSG_LEVEL_ERROR
                        col = QColor.fromRgb(255, 200, 200)
                    elif level==10: # MSG_LEVEL_STATE_CHANGE
                        col = QColor.fromRgb(200, 255, 200)
                    elif level==12: # MSG_LEVEL_WARNING
                        col = QColor.fromRgb(255, 255, 100)
                    else: # MSG_LEVEL_STATUSREPORT
                        col = QColor.fromRgb(210, 210, 210)
                    sender_item = QTableWidgetItem(sender)
                    sender_item.setBackground(col)
                    self.table.setItem(self.next_index, 0, sender_item)
                    time, = unpack('I', msg[12:16])
                    time_item = QTableWidgetItem(format_time(time))
                    time_item.setBackground(col)
                    self.table.setItem(self.next_index, 1, time_item)
                    text = msg[16:n_bytes+3].decode(encoding='utf-8')
                    text_item = QTableWidgetItem(text)
                    text_item.setBackground(col)
                    self.table.setItem(self.next_index, 2, text_item)
                    rsi = msg[n_bytes+3]
                    rsi_item = QTableWidgetItem("%3d"%rsi)
                    rsi_item.setBackground(col)
                    self.table.setItem(self.next_index, 3, rsi_item)
                    # print("%3d"%level, sender, format_time(time), text)
                    self.table.setCurrentCell(self.next_index, 0)
                # if it is a ping response
                elif lsb == 136:
                    # print("ping received")
                    self.next_index = self.table.rowCount()
                    self.table.insertRow(self.next_index)
                    self.table.setRowCount(self.next_index+1)
                    level = 0
                    col = QColor.fromRgb(210, 210, 210)
                    sender_item = QTableWidgetItem("")
                    sender_item.setBackground(col)
                    self.table.setItem(self.next_index, 0, sender_item)
                    time_item = QTableWidgetItem("")
                    time_item.setBackground(col)
                    self.table.setItem(self.next_index, 1, time_item)
                    up_rsi = msg[5]
                    down_rsi = msg[6]
                    text = f'ping RSI up={up_rsi} down = {down_rsi}'
                    text_item = QTableWidgetItem(text)
                    text_item.setBackground(col)
                    self.table.setItem(self.next_index, 2, text_item)
                    # the RSI is appended after the counted payload bytes
                    rsi = msg[n_bytes+3]
                    rsi_item = QTableWidgetItem("%3d"%rsi)
                    rsi_item.setBackground(col)
                    self.table.setItem(self.next_index, 3, rsi_item)
                    self.table.setCurrentCell(self.next_index, 0)

    def clear_list(self):
        """
        Clear the list of received messages.
        """
        self.table.setRowCount(0)

class MissionControlView(QWidget):
    
    def __init__(self, main_window):
        QWidget.__init__(self)
        self.mv = main_window
        self.cv = main_window.cv
        layout = QGridLayout()
        layout.setAlignment(Qt.AlignLeft | Qt.AlignTop)
        ping_button = QPushButton("Ping")
        ping_button.setFixedSize(200,50)
        ping_button.clicked.connect(self.send_ping)
        layout.addWidget(ping_button, 0, 0)
        full_button = QPushButton("Motors full")
        full_button.setFixedSize(200,50)
        full_button.clicked.connect(self.motor_full)
        layout.addWidget(full_button, 1, 1)
        zero_button = QPushButton("Motors off")
        zero_button.setFixedSize(200,50)
        zero_button.clicked.connect(self.motor_off)
        layout.addWidget(zero_button, 1, 0)
        calsave_button = QPushButton("Save Calibration")
        calsave_button.setFixedSize(200,50)
        calsave_button.clicked.connect(self.calsave)
        layout.addWidget(calsave_button, 2, 0)
        self.setLayout(layout)

    def send_ping(self):
        if hasattr(self.cv, 'serial'):
            # assemple an empty command message header (3 payload bytes)
            msg_buffer = bytearray(b'\xCC\x87\x03')
            # generate a random 16-bit hash
            hash = random.randint(0,65535)
            msg_buffer.extend(hash.to_bytes(2,'big'))
            msg_buffer.extend([0])
            # transmit
            self.cv.serial.write(msg_buffer)
            line = "sent ping "
            for c in msg_buffer:
                line += (" %0.2X" % c)
            print(line)
        else:
            print('port not open.')

    def calc_crc(self, payload):
        """
        Calculate a simple checksum over the payload.
        If one later on calculates the checksum over the payload plus this checksum, zero must be obtained.
        """
        crc = 0
        for byte in payload: crc ^= byte
        return crc
        
    def motor_off(self):
        if hasattr(self.cv, 'serial'):
            # assemple a command message header (4 payload bytes)
            msg_buffer = bytearray(b'\xCC\x86')
            payload = bytearray(b'MOFF')
            crc = self.calc_crc(payload)
            payload.extend([crc])
            # transmit
            msg_buffer.extend([len(payload)])
            msg_buffer.extend(payload)
            self.cv.serial.write(msg_buffer)
            line = "sent command "
            for c in msg_buffer:
                line += (" %0.2X" % c)
            print(line)
        else:
            print('port not open.')

    def motor_full(self):
        if hasattr(self.cv, 'serial'):
            # assemple a command message header (4 payload bytes)
            msg_buffer = bytearray(b'\xCC\x86')
            payload = bytearray(b'MFULL')
            crc = self.calc_crc(payload)
            payload.extend([crc])
            # transmit
            msg_buffer.extend([len(payload)])
            msg_buffer.extend(payload)
            self.cv.serial.write(msg_buffer)
            line = "sent command "
            for c in msg_buffer:
                line += (" %0.2X" % c)
            print(line)
        else:
            print('port not open.')

    def calsave(self):
        if hasattr(self.cv, 'serial'):
            # assemple a command message header (4 payload bytes)
            msg_buffer = bytearray(b'\xCC\x86')
            payload = bytearray(b'CALSAVE')
            crc = self.calc_crc(payload)
            payload.extend([crc])
            # transmit
            msg_buffer.extend([len(payload)])
            msg_buffer.extend(payload)
            self.cv.serial.write(msg_buffer)
            line = "sent command "
            for c in msg_buffer:
                line += (" %0.2X" % c)
            print(line)
        else:
            print('port not open.')


class MainWindow(QWidget):
    
    def __init__(self, rect):
        QWidget.__init__(self)
        self.app = app
        self.setWindowTitle("TAROS ground control station")
        self.setGeometry(100,100,0.6*rect.width(),0.8*rect.height())
        layout = QGridLayout()
        self.setLayout(layout)
        tabwidget = QTabWidget()
        label1 = QLabel("Widget in Tab 1.")
        tabwidget.addTab(label1, "Primary Flight Display")
        self.cv = CommunicationsView(self)
        tabwidget.addTab(self.cv, "Communications")
        self.mcv = MissionControlView(self)
        tabwidget.addTab(self.mcv, "Mission Control")
        layout.addWidget(tabwidget, 0, 0)

if __name__ == '__main__':
    # Execute when the module is not initialized from an import statement.

    # You need one (and only one) QApplication instance per application.
    # Pass in sys.argv to allow command line arguments for your app.
    # If you know you won't use command line arguments QApplication([]) works too.
    # create or reuse app object
    if not QtWidgets.QApplication.instance():
        app = QtWidgets.QApplication([])
    else:
        app = QtWidgets.QApplication.instance()
    # QGuiApplication.primaryScreen().availableGeometry()

    # Create a Qt widget, which will be our window.
    window = MainWindow(app.primaryScreen().availableGeometry())
    window.show()  # IMPORTANT!!!!! Windows are hidden by default.

    # Start the event loop.
    app.exec()

    # Your application won't reach here until you exit and the event
    # loop has stopped.

    if window.cv.port_available:
        window.cv.serial.close()
