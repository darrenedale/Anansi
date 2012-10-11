<html><head><title>Test Page</title></head><body><div>
<?php

echo '<form method="POST" action="test.php5">Text: <input name="TextField" type="text" value="' . $_POST['TextField'] . '" /><input type="submit" /></form>' . chr(10);

echo '<p>Query String: "' . $_SERVER['QUERY_STRING'] . '"</p>';
echo '<pre>';
echo '$_GET' . chr(10);
print_r($_GET);
echo chr(10);
echo '$_POST' . chr(10);
print_r($_POST);
echo chr(10);
echo '$_SERVER' . chr(10);
print_r($_SERVER);
echo chr(10);
echo '$_ENV' . chr(10);
print_r($_ENV);
echo chr(10);
echo '</pre>';

?>
</div></body></html>