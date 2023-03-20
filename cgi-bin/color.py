#!/usr/bin/env python3

import os

# получаем значение переменной "color" из переменных среды
color = os.environ.get('QUERY_STRING')
color = color.split('=')[1] if color else None

# если переменная "color" не задана, выводим сообщение об ошибке
if not color:
    print("Content-Type: text/plain")
    print()
    print("Ошибка: цвет не задан")
else:
    # выводим заголовок HTTP-ответа
    print("HTTP/1.1 200 OK")
    print("Content-Type: text/html")
    print("Content-Length: {}\r\n".format(len(color)))
    print()
    # выводим тело ответа
    print("<!DOCTYPE html>")
    print("<html>")
    print("<head>")
    print("<meta charset='UTF-8'>")
    print("<title>Выбранный цвет</title>")
    print("</head>")
    print("<body style='background-color: {};'>".format(color))
    print("<h1>Вы выбрали цвет {}</h1>".format(color))
    print("</body>")
    print("</html>")
