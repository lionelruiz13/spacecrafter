<html>
<head>
  <title>Spacecrafter pilot - Emmanuel Gaulon (Achères)</title>
  <link rel="shortcut icon" href="#">
  <style type="text/css" media="screen">
	.unstyled-button {
  	border: none;
  	padding: 0;
  	background: none;
	}
  </style>
</head>
<body>

<?php

function send_msg($msg)
 {
 	$socket = fsockopen("localhost", "7805", $errno, $errstr);
  	fputs($socket, $msg);
  	fclose($socket);
 }

if(isset($_POST["mybutton"])) 
{
   //print_r($_POST);
   
   $command = $_POST["mybutton"];
   //echo "commande = " .$command;
   send_msg($command);
   header('Location:index.php');
}
?>

<form name="new user" method="post"> 
    <button class="unstyled-button" title="flag fog on" type="submit" name="mybutton" value="flag fog on"><img src="a1.png"width="50" height="50"></button>
    <button class="unstyled-button" title="flag fog off" type="submit" name="mybutton" value="flag fog off"><img src="a2.png"></button>
    <button class="unstyled-button" title="script 01" type="submit" name="mybutton" value="script filename fscripts/01.sts action play"><img src="a2.png"></button>
</form>
</body>
</html>
