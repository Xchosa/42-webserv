<?php
// Test script for cookie handling (send Set-Cookie, read cookies, delete)

$message = "";

if (isset($_GET["set"]) && strpos($_GET["set"], "=") !== false)
{
    list($key, $value) = explode("=", $_GET["set"], 2);
    $key = trim($key);
    if ($key !== "")
    {
        setcookie($key, $value, time() + 3600, "/");
        $message = "Cookie set: " . htmlspecialchars($key, ENT_QUOTES, "UTF-8")
            . " = " . htmlspecialchars($value, ENT_QUOTES, "UTF-8");
    }
}
elseif (isset($_GET["delete"]))
{
    $key = trim($_GET["delete"]);
    if ($key !== "")
    {
        setcookie($key, "", time() - 3600, "/");
        $message = "Cookie deleted: " . htmlspecialchars($key, ENT_QUOTES, "UTF-8");
    }
}

header("Content-Type: text/html; charset=utf-8");

$rows = "";
foreach ($_COOKIE as $key => $value)
{
    $rows .= "<tr><td>" . htmlspecialchars($key, ENT_QUOTES, "UTF-8")
        . "</td><td>" . htmlspecialchars($value, ENT_QUOTES, "UTF-8") . "</td></tr>\n";
}
if ($rows === "")
    $rows = "<tr><td colspan=\"2\"><i>no cookies received</i></td></tr>\n";
?>
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>Cookie Test CGI</title>
</head>
<body>
  <h1>Cookie Test</h1>

  <?php if ($message !== "") : ?>
    <p><b><?= $message ?></b> (only visible in $_COOKIE on the next request)</p>
  <?php endif; ?>

  <h2>Cookies received from the client (HTTP_COOKIE)</h2>
  <table border="1">
    <tr><th>Name</th><th>Value</th></tr>
    <?= $rows ?>
  </table>

  <h2>Actions</h2>
  <ul>
    <li><a href="?set=testcookie=hello123">Set cookie "testcookie=hello123"</a></li>
    <li><a href="?delete=testcookie">Delete cookie "testcookie"</a></li>
  </ul>

  <h2>Set a custom cookie</h2>
  <form method="get">
    <input type="hidden" name="set" value="">
    <label>Name: <input type="text" name="key" onchange="document.querySelector('input[name=set]').value=this.value+'='+document.querySelector('input[name=val]').value"></label>
    <label>Value: <input type="text" name="val" onchange="document.querySelector('input[name=set]').value=document.querySelector('input[name=key]').value+'='+this.value"></label>
    <button type="submit" onclick="var f=this.form; f.set.value=f.key.value+'='+f.val.value;">Set</button>
  </form>
</body>
</html>
