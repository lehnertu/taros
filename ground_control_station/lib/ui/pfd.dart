import 'dart:math';
import 'package:flutter/material.dart';

class PFDpage extends StatelessWidget {
  const PFDpage({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Row(
      children: const <Widget>[
        Expanded(
          flex: 2,
          child: PFD(),
        ),
        Expanded(
          child: Text('navigation', textAlign: TextAlign.center),
        ),
      ],
    );
  }
}

class PFD extends StatelessWidget {
  const PFD({Key? key}) : super(key: key);

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
                  child: CustomPaint(
                    painter: PositionPainter(),
                  ),
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
              painter: AltitudePainter(),
            ),
          ),
        ),
      ],
    );
  }
}

class AltitudePainter extends CustomPainter {
  final alt = 27.8;
  @override
  void paint(Canvas canvas, Size size) {
    final double leftEdge = (size.width * 0.5).round().toDouble();
    final double rightEdge = (leftEdge + 30).toDouble();
    final double pixelsPerMeter = size.height / 60.0;
    final double centerPixel = size.height * 0.5;

    // round a value to full 10s
    double round10(x) {
      return (10.0 * (0.1 * x).round().toDouble());
    }

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

    // outline of the scale bar
    canvas.drawRect(
        Rect.fromLTWH(leftEdge, 0.1 * size.height, 30, 0.8 * size.height),
        thinLine);
    for (var m = round10(alt - 25.0); m < round10(alt + 25.0); m += 10.0) {
      canvas.drawLine(
        Offset(leftEdge, scale(round10(m))),
        Offset(rightEdge, scale(round10(m))),
        thickLine,
      );
      // label the markers
      TextSpan ts = TextSpan(
        text: '$m',
        style: const TextStyle(color: Colors.black, fontSize: 16),
      );
      TextPainter tp = TextPainter(
          text: ts,
          textAlign: TextAlign.right,
          textDirection: TextDirection.ltr);
      tp.layout(minWidth: 40.0, maxWidth: 60);
      tp.paint(canvas, Offset(leftEdge - 50, scale(round10(m)) - 10));
    }
    for (var m = (alt - 25.0).round(); m < (alt + 25.0).round(); m += 1) {
      canvas.drawLine(
        Offset(leftEdge + 5, scale(m)),
        Offset(rightEdge - 5, scale(m)),
        thinLine,
      );
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
      text: '$alt',
      style: const TextStyle(
          color: Colors.black, fontSize: 30, fontWeight: FontWeight.bold),
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
