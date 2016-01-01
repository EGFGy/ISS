#!/bin/bash
content_max=256
if !(( $CONTENT_LENGTH < $content_max ));then
	echo "ERROR: content too long"
	exit 0
fi
if [ "$REQUEST_METHOD" = "POST" -a ! -z "$CONTENT_LENGTH" ]; then
	read -n $CONTENT_LENGTH POST_data
else
	echo "ERROR: REQUEST_METHOD not correct"
	exit 0
fi


printf "Content-type: text/html\n\n"

cat <<EOF

<!DOCTYPE html>
<head>
	<title>unbenannt</title>
	<meta http-equiv="content-type" content="text/html;charset=utf-8" />
	<link rel="stylesheet" href="./test.css">
</head>

<body>
<h1>PÜÜÜÜÜÜÜÜÜÜÜÜÜÜ</h1>
$POST_data
<br>Content L: $CONTENT_LENGTH
<br>req. method: $REQUEST_METHOD


</body>

</html>
EOF
