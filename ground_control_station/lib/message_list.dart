import 'package:flutter/material.dart';

class Item {
  Item({
    required this.id,
    required this.name,
    required this.price,
    required this.description,
  });

  int id;
  String name;
  double price;
  String description;
}

class TableExample extends StatefulWidget {
  const TableExample({Key? key}) : super(key: key);
  @override
  State<StatefulWidget> createState() {
    return TableExampleState();
  }
}

class TableExampleState extends State<TableExample> {
  List<Item> _items = [];

  @override
  void initState() {
    super.initState();
    setState(() {
      _items = _generateItems();
    });
  }

  List<Item> _generateItems() {
    return List.generate(10, (int index) {
      return Item(
        id: index,
        name: 'Item $index',
        price: index * 1000.00,
        description: 'Details of item $index',
      );
    });
  }

  TableRow _buildTableRow(Item item) {
    return TableRow(
        key: ValueKey(item.id),
        decoration: const BoxDecoration(
          color: Colors.lightBlueAccent,
        ),
        children: [
          TableCell(
            verticalAlignment: TableCellVerticalAlignment.bottom,
            child: SizedBox(
              height: 50,
              child: Center(
                child: Text(item.id.toString()),
              ),
            ),
          ),
          TableCell(
            verticalAlignment: TableCellVerticalAlignment.middle,
            child: Padding(
              padding: const EdgeInsets.all(5),
              child: Text(item.name),
            ),
          ),
          TableCell(
            verticalAlignment: TableCellVerticalAlignment.middle,
            child: Padding(
              padding: const EdgeInsets.all(5),
              child: Text(item.price.toString()),
            ),
          ),
          TableCell(
            child: Padding(
              padding: const EdgeInsets.all(5),
              child: Text(item.description),
            ),
          ),
        ]);
  }

  @override
  Widget build(BuildContext context) {
    return Table(
      // border: TableBorder(
      //     bottom: BorderSide(color: Colors.red, width: 2),
      //     horizontalInside: BorderSide(color: Colors.red, width: 2),
      // ),
      // border: TableBorder.all(color: Colors.red, width: 2),
      border: TableBorder.symmetric(
        inside: const BorderSide(color: Colors.blue, width: 2),
        outside: const BorderSide(color: Colors.red, width: 5),
      ),
      defaultVerticalAlignment: TableCellVerticalAlignment.baseline,
      defaultColumnWidth: const IntrinsicColumnWidth(),
      columnWidths: const <int, TableColumnWidth>{
        0: FixedColumnWidth(20),
        1: FlexColumnWidth(3),
        2: MaxColumnWidth(FlexColumnWidth(2), FractionColumnWidth(0.3)),
        3: FixedColumnWidth(150),
      },
      // textDirection: TextDirection.rtl,
      textBaseline: TextBaseline
          .alphabetic, // Pass this argument when using [TableCellVerticalAlignment.fill]
      children: _items.map((item) => _buildTableRow(item)).toList(),
    );
  }
}
