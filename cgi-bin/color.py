import os
import urllib.parse

query_string = os.environ.get('QUERY_STRING')
color = None
if query_string:
    for param in query_string.split('&'):
        key, value = param.split('=')
        if key == 'color':
            color = urllib.parse.unquote(value).replace("%23", "#")
            break

html = """
<!DOCTYPE html>
<html>
<head>
    <title>Color Page</title>
</head>
<body style="background-color: {color}">
    <h1>Hello, World!</h1>
    <p>The background color of this page is {color}.</p>
</body>
</html>
""".format(color=color or 'white')

print('HTTP/1.1 200 OK')
print('Content-Length: ' + str(len(html)))
print('Content-Type: text/html')
print()
print(html)
