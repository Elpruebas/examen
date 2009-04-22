<?php if(empty($peliculas)):
        print "No hasdfasdfasdfasdsdfasdfasdfasdfasdfasdfy peliculas en estos momentos.";
      else: ?>
    <?php foreach($peliculas as $pelicula): ?>
        <?php print pelicula_view($pelicula, true, false)->content['pelicula']['#value'];?>
    <?php endforeach; ?>
<?php endif; ?>
