const char webpageCode[] =
R"=====(
<!DOCTYPE html>
  <html>
  <!------------------------------C S S--------------------------------->
    <head>
      <style>
      
      </style>
    </head>
  <!----------------------------H T M L--------------------------------->
    <body>
      <div align="center">
        <form action="/teste" method="POST">Digite um valor
          <br>
          Valor:<input type="text" name="valor" placeholder="VALOR">
          <br>
          <input type="submit" name="SUBMIT" value="Submit">
        </form>
        <br>
      <div align="center">
        <button><a href="/?LED=on">Liga</a></button>
        <button><a href="/?LED=off">Desliga</a></button>
      </div>
     </div>
    <!---------------------------JavaScript------------------------------->
      <script>
         console.log("OK")
      </script>
    </body>
  </html>
)=====";
