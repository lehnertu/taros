import 'package:flutter/material.dart';
import 'package:flutter_libserialport/flutter_libserialport.dart';

class ModemSelector extends StatefulWidget {
  const ModemSelector({Key? key}) : super(key: key);

  @override
  State<ModemSelector> createState() => _ModemSelectorState();
}

class _ModemSelectorState extends State<ModemSelector>
    with AutomaticKeepAliveClientMixin {
  String dropdownValue = 'n.a.';
  // this works !
  var availablePorts = ['n.a.'] + SerialPort.availablePorts;

  @override
  bool get wantKeepAlive => true;

  @override
  Widget build(BuildContext context) {
    super.build(context);
    // print(availablePorts);
    return DropdownButton<String>(
      value: dropdownValue,
      icon: const Icon(Icons.arrow_downward),
      elevation: 16,
      style: const TextStyle(color: Colors.deepPurple),
      underline: Container(
        height: 2,
        color: Colors.deepPurpleAccent,
      ),
      onChanged: (String? newValue) {
        setState(() {
          dropdownValue = newValue!;
        });
      },
      items: availablePorts.map<DropdownMenuItem<String>>((String value) {
        return DropdownMenuItem<String>(
          value: value,
          child: Text(value),
        );
      }).toList(),
    );
  }
}

/*
void main() {
  var availablePorts = SerialPort.availablePorts;
  print(availablePorts);

  if (availablePorts.isNotEmpty) {
    final address = availablePorts.first;
    print(address);

    final port = SerialPort(address);
    print(port.description);
    // final conf = port.config;
    // print(conf.baudRate);

    print('found port.');

    if (!port.openReadWrite()) {
      print(SerialPort.lastError);
      // exit(-1);
    } else {
      var cfg = port.config;
      print('config:');
      print(cfg.baudRate);
      print(cfg.bits);
      print(cfg.parity);
      print(cfg.stopBits);
      cfg.baudRate = 115200;
      cfg.bits = 8;
      cfg.stopBits = 1;
      port.config = cfg;

      // port.write(/* ... */);
      final reader = SerialPortReader(port);
      reader.stream.listen((data) {
        print(String.fromCharCodes(data));
      });
    }
  }
  print('done.');
}
*/
