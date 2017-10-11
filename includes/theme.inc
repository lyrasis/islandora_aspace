<?php

function islandora_aspace_preprocess_islandora_default_edit(array &$variables) {
  $object = $variables['islandora_object'];
  if(in_array(ISLANDORA_ASPACE_OBJECT_CMODEL, $object->models)) {
    $datastreams = array();
    foreach($object as $ds) {
      $datastreams[] = $ds->id;
    }

    $result = array_search(ISLANDORA_ASPACE_DSID, $datastreams);
    if($result !== FALSE) {
      $variables['datastream_table']['rows'][$result][6]['data'] = NULL;
      $variables['datastream_table']['rows'][$result][8]['data'] = NULL;
    }

    $result = array_search('MODS', $datastreams);
    if($result !== FALSE) {
      $variables['datastream_table']['rows'][$result][6]['data'] = NULL;
      $variables['datastream_table']['rows'][$result][8]['data'] = NULL;
    }

    $result = array_search('DC', $datastreams);
    if($result !== FALSE) {
      $variables['datastream_table']['rows'][$result][6]['data'] = NULL;
      $variables['datastream_table']['rows'][$result][8]['data'] = NULL;
    }
  }
}
