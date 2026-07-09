<?php
$body = <<<'HTML'
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>Color Buttons</title>
  <style>
    body {
      margin: 0;
      min-height: 100vh;
      display: grid;
      place-items: center;
      font-family: Arial, sans-serif;
      background: white;
      color: #202124;
    }

    main {
      text-align: center;
    }

    h1 {
      margin: 0 0 24px;
      font-size: 32px;
    }

    .buttons {
      display: flex;
      gap: 12px;
      justify-content: center;
    }

    button {
      border: 2px solid black;
      padding: 12px 18px;
      font-size: 16px;
      cursor: pointer;
      color: white;
      font-weight: 700;
    }

  </style>
</head>
<body>
  <main>
    <h1>Choose a background color</h1>
    <div class="buttons">
      <button type="button" data-color="#ff6b6b">Red</button>
      <button type="button" data-color="#4dabf7">Blue</button>
      <button type="button" data-color="#51cf66">Green</button>
    </div>
  </main>

  <script>
    const buttons = document.querySelectorAll("button[data-color]");

    buttons.forEach((button) => {
      button.style.backgroundColor = button.dataset.color;

      button.addEventListener("click", () => {
        document.body.style.background = button.dataset.color;
      });
    });
  </script>
</body>
</html>
HTML;

header("Content-Type: text/html; charset=utf-8");
echo $body;
?>
