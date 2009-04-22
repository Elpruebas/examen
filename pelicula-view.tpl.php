<?php if(!$pelicula['teaser']): ?>
  <div class="pelicula-seccion">
        <div class="pelicula-item">
            <pre><?php print_r($pelicula) ?></pre>
            <h2><?php print $pelicula['node']->title?></h2>
        </div>
  </div>
<?php else: ?>
   <div class="pelicula-seccion" style="float:left; padding:30px">
        <div class="pelicula-item">
            <h2><?php print l($pelicula['node']->title, "node/" . $pelicula['node']->nid) ?></h2>
        </div>
   </div>
<?php endif; ?>
