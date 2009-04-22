<?php
/**
 * Define la información sobre los peliculas.
 */
function _pelicula_node_info(){
  return array(
          'name' => t('Pelicula'),
          'module' => 'pelicula',
          'title_label' => t('pelicula'),
          'with_body' => False,
          'description' => t("De de alta una nueva pelicula."),
  );
}


/**
 * Implementation of hook_access().
 */
function pelicula_access($op, $node) {
  global $user;
  if ($op == 'create' || $op == 'delete' || $op == 'update'){
    //Solo se deja crear este tipo de nodos al administrador
    return user_access("administrar peliculas");
  }else {
    return user_access("access content");
  }
}

/**
 * Nodeapi para pelicula
 */
function _pelicula_nodeapi(&$node, $op, $teaser, $page) {
  switch ($op) {
    case 'delete revision':
      _pelicula_delete_revision($node);
      break;
  }
}

/**
 * Implementation of hook_form().
 */
function pelicula_form(&$node) {
  $type = node_get_types('type', $node);
  $form['#attributes']['enctype'] = 'multipart/form-data';
  
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => check_plain(t('Título de la Película')),
    '#required' => TRUE,
    '#default_value' => $node->title,
  );
  
  $form['body_filter']['body'] = array(
    '#type' => 'textarea',
    '#title' => check_plain(t('Descripción de la Película')),
    '#default_value' => $node->body,
    '#required' => FALSE
  );
  $form['body_filter']['format'] = filter_form($node->format);
    
  if(!$node->genero){
  	$node->genero = array();
	  $consulta_generos = db_query("SELECT * FROM peliculas_gen_inter where id_pelicula = %d", $node->nid);
	  while ($area = db_fetch_object($consulta_generos)){
	    $node->genero[$area->id_genero] = t($area->id_genero);
	  }  	
  }
  $form['genero'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#required' => TRUE,
    '#options' => _pelicula_opciones_genero(),
    '#size' => 4,
    '#title' => t('Genero/s de la película'),
    '#default_value' => $node->genero,
  );
  
  $form['pais'] = array(
      '#type' => 'textfield',
      '#title' => check_plain(t('País al que pertence la película')),      
      '#required' => False,
      '#default_value' => $node->pais,
  );
  
  $form['ano'] = array(
      '#type' => 'textfield',
      '#title' => check_plain(t('Año en el que se estrenó la Película')),      
      '#required' => true,
      '#default_value' => $node->ano,
  );
  
  $form['id_director'] = array(
    '#type' => 'select',
    '#multiple' => False,
    '#required' => False,
    '#options' => _pelicula_opciones_director(),
    '#size' => 1,
    '#title' => check_plain(t('Director de la Película')),
    '#default_value' => $node->id_director,
  );
  
  if(!$node->actores){
    $node->actores = array();
    $consulta_actores = db_query("SELECT * FROM peliculas_act_inter where id_pelicula = %d", $node->nid);
    while ($area = db_fetch_object($consulta_actores)){
      $node->actores[$area->id_actor] = t($area->id_actor);
    }   
  }
  $form['actores'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#required' => False,
    '#options' => _pelicula_opciones_actores(),
    '#size' => 4,
    '#title' => t('Actores de películas'),
    '#default_value' => $node->actores,
  );
  
  $form['mi_valoracion'] = array(
      '#type' => 'textfield',
      '#title' => check_plain(t('Mi valoración de la Película')),      
      '#required' => FALSE,
      '#default_value' => $node->mi_valoracion,
  );
    
  $form['critica_valoracion'] = array(
      '#type' => 'textfield',
      '#title' => check_plain(t('Valoración de la crítica de la Película')),      
      '#required' => False,
      '#default_value' => $node->critica_valoracion,
  );

  $form['almacen'] = array(
      '#type' => 'textfield',
      '#title' => check_plain(t('Almacenamiento de la pélicula. En caso de ser una película formato DVD poner 0, 
        en caso de ser formato DVDRip y estar grabada en un DVD poner el nº del DVD en el que está grabada, 
        si está grabada en CD poner -1')),      
      '#required' => True,
      '#default_value' => $node->almacen,
  );
  
  return $form;
}

/**
 * Implementation of hook_validate().
 */
function pelicula_validate($node, $form = NULL) {
  //print "<pre>"; print_r($form); print "</pre>";
	$query = db_query("SELECT title FROM peliculas p inner join node using(nid) where title = '%s'", $node->title);
  while($resultado = db_fetch_object($query)){
    $peliculas[$resultado->title]= $resultado->title;
  }
  if(!empty($peliculas) && strpos($_GET['q'], "edit") === FALSE){
    form_set_error('title', t('Ya existe una película con ese nombre.'));
  }elseif(!empty($peliculas) && $form['title']['#default_value'] != $node->title){
    form_set_error('title', t('Ya existe una película con ese nombre.'));
  }
  
	if($node->ano <= 1925 || $node->ano >=2100){
    form_set_error('ano', t('El año de la película es incorrecto.'));
  }
  if($node->critica_valoracion < 0 || $node->critica_valoracion >10){
    form_set_error('critica_valoracion', t('La valoracion personal es incorrecta. Debe ser mayor o igual a 0 y menor o igual a 10.'));
  }
  if($node->mi_valoracion < 0 || $node->mi_valoracion >10){
    form_set_error('mi_valoracion', t('La valoracion de la crítica es incorrecta. Debe ser mayor o igual a 0 y menor o igual a 10.'));
  }
  if($node->almacen < -1){
    form_set_error('almacen', t('No puede ser un valor menor de -1.'));
  }
}

/**
 * Implementation of hook_insert().
 */
function pelicula_insert($node) {
  db_query("INSERT INTO {peliculas} (vid, nid, id_director, mi_valoracion, critica_valoracion, ano, pais, almacen) 
  VALUES(%d,%d,%d,%d,%d,%d,'%s', %d)", $node->vid, $node->nid, $node->id_director, $node->mi_valoracion, 
  $node->critica_valoracion, $node->ano, $node->pais, $node->almacen);  
  _pelicula_insertar_genero($node);
  _pelicula_insertar_actores($node);
}

/**
 * Implementation of hook_update().
 *
 * As an existing node is being updated in the database, we need to do our own
 * database updates.
 */
function pelicula_update($node) {
  // if this is a new node or we're adding a new revision,
  if ($node->revision) {
    pelicula_insert($node);
  }
  else {
		db_query("UPDATE {peliculas} set id_director = %d, mi_valoracion = %d, 
    critica_valoracion = %d, ano = %d, pais = '%s', almacen = %d WHERE vid = %d and nid = %d",
    $node->director, $node->mi_valoracion, $node->critica_valoracion, $node->ano, $node->pais, 
    $node->almacen, $node->vid, $node->nid);
    db_query("DELETE FROM peliculas_gen_inter where id_pelicula = %d", $node->nid);
    db_query("DELETE FROM peliculas_act_inter where id_pelicula = %d", $node->nid);
    _pelicula_insertar_genero($node);
    _pelicula_insertar_actores($node);
  }
}

function _pelicula_insertar_genero($nodo = NULL){
  foreach ($nodo->genero as $genero){
	  db_query("INSERT INTO {peliculas_gen_inter} (id_pelicula, id_genero) 
	    VALUES(%d,%d)", $nodo->nid, $genero);
	}
}

function _pelicula_insertar_actores($nodo = NULL){
  foreach ($nodo->actores as $actor){
    db_query("INSERT INTO {peliculas_act_inter} (id_pelicula, id_actor) 
      VALUES(%d,%d)", $nodo->nid, $actor);
  }
}

/**
 * Implementation of hook_delete().
 */
function pelicula_delete($node) {
  db_query('DELETE FROM {peliculas} WHERE nid = %d', $node->nid);
  db_query('DELETE FROM {peliculas_gen_inter} WHERE id_pelicula = %d', $node->nid);
  db_query('DELETE FROM {peliculas_act_inter} WHERE id_pelicula = %d', $node->nid);
}

/**
 * Eliminar revisión (se utiliza desde _pelicula_nodeapi).
 */
function _pelicula_delete_revision($node) {
  db_query('DELETE FROM {peliculas} WHERE vid = %d', $node->vid);
  db_query('DELETE FROM {peliculas_gen_inter} WHERE id_pelicula = %d', $node->vid);
  db_query('DELETE FROM {peliculas_act_inter} WHERE id_pelicula = %d', $node->vid);
}

/**
 * Implementation of hook_load().
 */
function pelicula_load($node) {
  $additions = db_fetch_object(db_query('SELECT * FROM {peliculas} WHERE vid = %d', $node->vid));
  return $additions;
}

/**
 * Implementation of hook_view
 */
function pelicula_view($node, $teaser = FALSE, $page = FALSE) {
  $params['teaser'] = $teaser;
  $params['page'] = $page;  
  $node = node_prepare($node, $teaser);
  $node->content['pelicula'] = array(
    '#value' => theme('pelicula_view', _pelicula_get_view_params($node, $params)),
    '#weight' => 1,
  );
  return $node;
}

/**
 * Funcion que devuelve los parametros necesarios para mostrar el nodo pelicula
 *
 * @param unknown_type $node
 * @return unknown
 */
function _pelicula_get_view_params($node, $params){
  $params['nid'] = $node->nid;
  $params['nombre'] = $node->title;
  $consulta_actores = db_query("SELECT title FROM peliculas_act_inter p inner join node n on p.id_actor = n.nid where id_pelicula = %d", $node->nid);
  while ($actor = db_fetch_object($consulta_actores)){
    $actores[$actor->title] = t($actor->title);
  }
  $node->actores = $actores;
  $consulta_generos = db_query("SELECT title FROM peliculas_gen_inter p inner join node n on p.id_genero = n.nid where id_pelicula = %d", $node->nid);
  while ($genero = db_fetch_object($consulta_generos)){
    $generos[$genero->title] = t($genero->title);
  }
  $node->genero = $generos;
  $params['node'] = $node;
  return $params;
}

// --------------------------------------------------- Funciones con los datos de los seleccionables

function _pelicula_opciones_genero() {
  $generos = array();
  $generos[''] = t('- Please choose -');
  $consulta_generos = db_query("SELECT * FROM node n where type = 'genero' order by title");
  while ($area = db_fetch_object($consulta_generos)){
    $generos[$area->nid] = t($area->title);
  }
  
  return $generos;
}

function _pelicula_opciones_director() {
  $directores = array();
  $directores[''] = t('- Please choose -');
  $consulta_directores = db_query("SELECT * FROM node n where type = 'directores' order by title");
  while ($area = db_fetch_object($consulta_directores)){
    $directores[$area->nid] = t($area->title);
  }
  
  return $directores;
}

function _pelicula_opciones_actores() {
  $actores = array();
  $actores[''] = t('- Please choose -');
  $consulta_actores = db_query("SELECT * FROM node n where type = 'actores' order by title");
  while ($area = db_fetch_object($consulta_actores)){
    $actores[$area->nid] = t($area->title);
  }
  
  return $actores;
}
