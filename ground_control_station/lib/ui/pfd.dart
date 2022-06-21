// import 'dart:math';
import 'package:flutter/material.dart';

class PFDpage extends StatelessWidget {
  PFDpage({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        Expanded(
          flex: 2,
          child: PFD(),
        ),
        Expanded(
          flex: 1,
          child: Text('Navigation'),
        ),
      ],
    );
  }
}

class PFD extends StatefulWidget {
  PFD({Key? key}) : super(key: key);
  @override
  StatePFD createState() => StatePFD();
}

class StatePFD extends State<PFD> {
  var _altimeter = 23.5;

  @override
  void initState() {
    super.initState();
  }

  @override
  void dispose() {
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Row(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Expanded(
          flex: 1,
          // here goes the velocity display
          child: Container(
            color: const Color.fromARGB(255, 233, 255, 68),
          ),
        ),
        Expanded(
          flex: 3,
          child: Column(
            children: [
              Expanded(
                flex: 1,
                // here goes the compass
                child: Container(
                  color: const Color.fromARGB(255, 252, 68, 255),
                ),
              ),
              Expanded(
                flex: 4,
                child: Container(
                  color: const Color.fromARGB(255, 84, 255, 68),
                  child: Slider(
                    value: _altimeter,
                    min: -30.0,
                    max: 150.0,
                    label: _altimeter.toString(),
                    onChanged: (value) {
                      setState(() {
                        _altimeter = value;
                      });
                    },
                  ),
                  // child: CustomPaint(
                  //   painter: PositionPainter(),
                  // ),
                ),
              ),
            ],
          ),
        ),
        Expanded(
          flex: 1,
          // here goes the altitude display
          child: Container(
            color: const Color.fromARGB(255, 196, 244, 255),
            child: CustomPaint(
              painter: AltitudePainter(_altimeter),
            ),
          ),
        ),
      ],
    );
  }
}

class AltitudePainter extends CustomPainter {
  final double alt;
  AltitudePainter(this.alt);

  @override
  void paint(Canvas canvas, Size size) {
    final double leftEdge = (size.width * 0.5).round().toDouble();
    final double rightEdge = (leftEdge + 30).toDouble();
    final double pixelsPerMeter = size.height / 60.0;
    final double centerPixel = size.height * 0.5;

    // pixel scale of altitude centered about alt
    double scale(meters) {
      return (alt - meters) * pixelsPerMeter + centerPixel;
    }

    // paint the scale
    final thinLine = Paint()
      ..style = PaintingStyle.stroke
      ..strokeWidth = 2.0
      ..color = const Color.fromARGB(255, 0, 0, 0);
    final thickLine = Paint()
      ..style = PaintingStyle.stroke
      ..strokeWidth = 4.0
      ..color = const Color.fromARGB(255, 0, 0, 0);
    // background of the scale bar
    if (0.0 > alt - 25.0) {
      final groundBox = Paint()
        ..style = PaintingStyle.fill
        ..strokeWidth = 0
        ..color = Color.fromARGB(255, 208, 150, 57);
      canvas.drawRect(
          Rect.fromLTWH(
              leftEdge, scale(0.0), 30, 0.9 * size.height - scale(0.0)),
          groundBox);
    }
    // outline of the scale bar
    canvas.drawRect(
        Rect.fromLTWH(leftEdge, 0.1 * size.height, 30, 0.8 * size.height),
        thinLine);
    for (var m = (alt - 23).round(); m < (alt + 24).round(); m += 1) {
      if (m % 10 == 0) {
        canvas.drawLine(
          Offset(leftEdge, scale(m)),
          Offset(rightEdge, scale(m)),
          thickLine,
        );
        // label the markers
        TextSpan ts = TextSpan(
          text: m.toStringAsFixed(0),
          style: const TextStyle(color: Colors.black, fontSize: 16),
        );
        TextPainter tp = TextPainter(
            text: ts,
            textAlign: TextAlign.right,
            textDirection: TextDirection.ltr);
        tp.layout(minWidth: 40.0, maxWidth: 60);
        tp.paint(canvas, Offset(leftEdge - 50, scale(m) - 10));
      } else if (m % 5 == 0) {
        canvas.drawLine(
          Offset(leftEdge + 5, scale(m)),
          Offset(rightEdge - 5, scale(m)),
          thinLine,
        );
      } else {
        canvas.drawLine(
          Offset(leftEdge + 10, scale(m)),
          Offset(rightEdge - 10, scale(m)),
          thinLine,
        );
      }
    }

    // the numeric altitude display
    final altBox = Paint()
      ..style = PaintingStyle.fill
      ..strokeWidth = 2.0
      ..color = Colors.white;
    canvas.drawRect(
        Rect.fromLTWH(leftEdge - 81, centerPixel - 20, 80, 40), altBox);
    final altOutline = Paint()
      ..style = PaintingStyle.stroke
      ..strokeWidth = 5.0
      ..color = Colors.black;
    canvas.drawRect(
        Rect.fromLTWH(leftEdge - 81, centerPixel - 20, 80, 40), altOutline);
    TextSpan ts = TextSpan(
      text: alt.toStringAsFixed(0),
      style: const TextStyle(
        color: Colors.black,
        fontSize: 30,
        fontFamily: 'Monospace',
        fontWeight: FontWeight.bold,
      ),
    );
    TextPainter tp = TextPainter(
        text: ts, textAlign: TextAlign.right, textDirection: TextDirection.ltr);
    tp.layout(minWidth: 70.0, maxWidth: 70);
    tp.paint(canvas, Offset(leftEdge - 75, centerPixel - 18));
  }

  @override
  bool shouldRepaint(AltitudePainter oldDelegate) => true;
}

class PositionDisplay extends StatelessWidget {
  const PositionDisplay({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Container(
      color: Colors.yellow[100],
      padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 20),
      child: CustomPaint(
        painter: PositionPainter(),
      ),
    );
  }
}

class PositionPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {}

  @override
  bool shouldRepaint(PositionPainter oldDelegate) {
    return true;
  }
}
