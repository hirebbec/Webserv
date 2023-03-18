import datetime

response_body = """
<!DOCTYPE html>
<html>
<head>
	<title>Текущее время</title>
	<style>
		body {{
			background-color: #D3D3D3;
			text-align: center;
		}}
		h1 {{
			margin-top: 30vh;
			font-size: 5rem;
			color: #333;
		}}
	</style>
</head>
<body>
	<h1>{}</h1>
</body>
</html>
""".format(datetime.datetime.now())

# Формируем HTTP ответ с заголовками
response = "HTTP/1.1 200 OK\r\n"
response += "Content-Length: {}\r\n".format(len(response_body))
response += "Content-Type: text/html\r\n"
response += "\r\n"
response += response_body

# Выводим HTTP ответ в консоль
print(response)
