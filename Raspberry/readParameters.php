<?php
define('DB_SERVER', 'localhost');
define('DB_USERNAME', 'mysql_user');
define('DB_PASSWORD', 'serwerek@123');
define('DB_NAME', 'CO');

date_default_timezone_set('Europe/Warsaw');

$date = date("Y-m-d H:i:s");

$sql_conn = mysqli_connect(DB_SERVER, DB_USERNAME, DB_PASSWORD, DB_NAME);

if (!$sql_conn) {
    die("Connection failed: " .mysqli_connect_error());
}

$temperature_now = $_POST["temperature_now"];
$temperature_set = $_POST["temperature_set"];
$PID_status = $_POST["PID_status"];
$manual_mode = $_POST["manual_mode"];

$inserting = "INSERT INTO Logs (DATE, TEMPERATURE_NOW, TEMPERATURE_SET, PID_STATUS, MANUAL_MODE) 
VALUES ('{$date}', '{$temperature_now}', '{$temperature_set}', '{$PID_status}', '{$manual_mode}')";

if(mysqli_query($sql_conn, $inserting) > 0) {
    echo "OK written data";
} else {
    die("Error writing data " . mysqli_error($sql_conn));
}

mysqli_close($sql_conn);
