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

$zapytanie = "SELECT * FROM Config ORDER BY ID DESC LIMIT 1";
$idzapytania = mysqli_query($sql_conn, $zapytanie);
$row1 = mysqli_fetch_assoc($idzapytania);

$zapytanie = "SELECT * FROM Logs ORDER BY ID DESC LIMIT 1";
$idzapytania = mysqli_query($sql_conn, $zapytanie);
$row2 = mysqli_fetch_assoc($idzapytania);

$lastID = $row1["ID"];
$tempSetOnController = $row2["TEMPERATURE_SET"];
$tempSetOnServer = $row1["TEMPERATURE_SET"];
$manualModeSetOnController = $row2["MANUAL_MODE"];
$manualModeSetOnServer = $row1["MANUAL_MODE"];
$pidStatusOnController = $row2["PID_STATUS"];
$pidSetOnServer = $row1["PID_SET"];

if(($manualModeSetOnServer == $manualModeSetOnController) && ($manualModeSetOnServer == 1) && ($pidSetOnServer == $pidStatusOnController)) {
    $zapytanie = "UPDATE Config SET CONFIRMED='{$date}' WHERE ID='{$lastID}'";

    if(mysqli_query($sql_conn, $zapytanie) > 0) {
        echo "OK written confirmation";
    } else {
        die("Error writing confirmation");
    }
}

if(($manualModeSetOnServer == $manualModeSetOnController) && ($manualModeSetOnServer == 0) && ($tempSetOnServer = $tempSetOnController)) {
    $zapytanie = "UPDATE Config SET CONFIRMED='{$date}' WHERE ID='{$lastID}'";

    if (mysqli_query($sql_conn, $zapytanie) > 0) {
        echo "OK written confirmation";
    } else {
        die("Error writing confirmation");
    }
}

mysqli_close($sql_conn);