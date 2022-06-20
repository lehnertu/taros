import 'package:flutter/material.dart';
import 'modem.dart';
import 'message_list.dart';
import 'ui/pfd.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'TAROS Ground Control',
      theme: ThemeData(
        // This is the theme of your application.
        // You'll see the application has a blue toolbar.
        primarySwatch: Colors.blue,
      ),
      home: _TabsMainWidget(),
    );
  }
}

class _TabsMainWidget extends StatefulWidget {
  @override
  _TabsMainWidgetState createState() => _TabsMainWidgetState();
}

class _TabsMainWidgetState extends State<_TabsMainWidget>
    // A ticker invokes a call-back once per animation frame.
    // This one provides a ticker that only ticks while the current tree is active.
    with
        SingleTickerProviderStateMixin {
  TabController? _tabController;

  @override
  void initState() {
    _tabController = TabController(
      initialIndex: 0,
      length: 3,
      vsync: this,
    );
    _tabController!.addListener(() {
      setState(() {});
    });
    super.initState();
  }

  @override
  void dispose() {
    _tabController!.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final tabs = ['PFD', 'system messages', 'settings'];
    const ms = ModemSelector();

    return Scaffold(
      appBar: AppBar(
        automaticallyImplyLeading: false,
        title: const Center(
          child: Text(
            'TAROS ground control',
            textScaleFactor: 1.2,
          ),
        ),
        bottom: TabBar(
          controller: _tabController,
          indicatorWeight: 5, // height of the colored indicator
          isScrollable:
              true, // this also collapses the width of the tabs to the necessary minimum
          tabs: [
            for (final tab in tabs) Tab(text: tab),
          ],
        ),
      ),
      body: TabBarView(
        controller: _tabController,
        children: const [
          Center(
            child: PFDpage(),
          ),
          TableExample(),
          ms,
        ],
      ),
    );
  }
}
