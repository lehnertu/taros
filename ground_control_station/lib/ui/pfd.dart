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
        const Expanded(
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
        Flexible(
          flex: 1,
          // here goes the altitude display
          child: Container(
            color: const Color.fromARGB(255, 168, 168, 168),
            constraints: BoxConstraints(
              minWidth: 150,
              maxWidth: double.infinity,
              minHeight: 500,
              maxHeight: double.infinity,
            ),
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
    final double leftEdge = (size.width * 0.6).round().toDouble();
    final double rightEdge = (leftEdge + 30).toDouble();
    final double bottomEdge = (0.9 * size.height).toDouble();
    final double topEdge = 0.1 * size.height.toDouble();
    final double pixelsPerMeter = (bottomEdge - topEdge) / 50.0;
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
    final gBoxTop = scale(0.0) > topEdge ? scale(0.0) : topEdge;
    if (0.0 > alt - 25.0) {
      final groundBox = Paint()
        ..style = PaintingStyle.fill
        ..strokeWidth = 0
        ..color = const Color.fromARGB(255, 208, 150, 57);
      canvas.drawRect(
          Rect.fromLTWH(leftEdge, gBoxTop, 30, 0.9 * size.height - gBoxTop),
          groundBox);
    }
    final yBoxBottom = scale(0.0) < bottomEdge ? scale(0.0) : bottomEdge;
    final yBoxTop = scale(20.0) > topEdge ? scale(20.0) : topEdge;
    if ((yBoxTop < bottomEdge) & (yBoxBottom > topEdge)) {
      final groundBox = Paint()
        ..style = PaintingStyle.fill
        ..strokeWidth = 0
        ..color = const Color.fromARGB(255, 253, 255, 124);
      canvas.drawRect(
          Rect.fromLTWH(leftEdge, yBoxTop, 30, yBoxBottom - yBoxTop),
          groundBox);
    }
    final bBoxBottom = scale(20.0) < bottomEdge ? scale(20.0) : bottomEdge;
    if (bBoxBottom > topEdge) {
      final groundBox = Paint()
        ..style = PaintingStyle.fill
        ..strokeWidth = 0
        ..color = const Color.fromARGB(255, 196, 244, 255);
      canvas.drawRect(
          Rect.fromLTWH(leftEdge, topEdge, 30, bBoxBottom - topEdge),
          groundBox);
    }
    // outline of the scale bar
    canvas.drawRect(
        Rect.fromLTWH(leftEdge, 0.1 * size.height, 30, 0.8 * size.height),
        thinLine);
    for (var m = (alt - 24.5).round(); m < (alt + 25.5).round(); m += 1) {
      if (m % 10 == 0) {
        canvas.drawLine(
          Offset(leftEdge, scale(m)),
          Offset(rightEdge, scale(m)),
          thickLine,
        );
        // label the markers
        TextSpan ts = TextSpan(
          text: m.toStringAsFixed(0),
          style: const TextStyle(color: Colors.black, fontSize: 14),
        );
        TextPainter tp = TextPainter(
            text: ts,
            textAlign: TextAlign.right,
            textDirection: TextDirection.ltr);
        tp.layout(minWidth: 40.0, maxWidth: 60);
        tp.paint(canvas, Offset(leftEdge - 50, scale(m) - 9));
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
        Rect.fromLTWH(leftEdge - 71, centerPixel - 16, 70, 32), altBox);
    final altOutline = Paint()
      ..style = PaintingStyle.stroke
      ..strokeWidth = 5.0
      ..color = Colors.black;
    canvas.drawRect(
        Rect.fromLTWH(leftEdge - 71, centerPixel - 16, 70, 32), altOutline);
    TextSpan ts = TextSpan(
      text: alt.toStringAsFixed(0),
      style: const TextStyle(
        color: Colors.black,
        fontSize: 24,
        fontFamily: 'Monospace',
        fontWeight: FontWeight.bold,
      ),
    );
    TextPainter tp = TextPainter(
        text: ts, textAlign: TextAlign.right, textDirection: TextDirection.ltr);
    tp.layout(minWidth: 65.0, maxWidth: 65);
    tp.paint(canvas, Offset(leftEdge - 71, centerPixel - 14));
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
