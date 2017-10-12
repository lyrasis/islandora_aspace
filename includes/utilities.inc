<?php

/**
 * @file
 * This file contains utilitiy functions.
 */

function islandora_aspace_transform_document($xml_string, $transform_path, $format = TRUE) {
  if($transform_path == 'No Transform') {
    return $xml_string;
  }

  $dom = new DOMDocument();
  $dom->loadXML($xml_string);

  $xsl = new DOMDocument();
  $xsl->load($transform_path);

  $xslt = new XSLTProcessor();
  $xslt->importStyleSheet($xsl);

  $dom = $xslt->transformToDoc($dom);
  $dom->formatOutput = $format;

  return $dom->saveXML();
}

function islandora_aspace_preprocess_aspace_data($data) {
  module_load_include('inc', 'islandora_aspace', 'includes/archivesspace');

  $resolveref = function(&$array) use(&$resolveref) {
    if(isset($array['_resolved'])) {
      $item = $array['_resolved'];
      unset($array['_resolved']);
      $array = array_merge($array, $item);
    }
    foreach($array as $key => $item) {
      if(is_array($item)) {
        $resolveref($array[$key]);
      }
    }
  };
  $filter = function(&$array, $keys) use(&$filter) {
    $removal_list = array();
    foreach($array as $key => $item) {
      if(in_array($key, $keys, TRUE)){
        $removal_list[] = $key;
      }
      elseif(is_array($item)) {
        $filter($array[$key], $keys);
      }
    }
    foreach($removal_list as $key) {
      unset($array[$key]);
    }
  };
  $keys = array(
    'lock_version',
    'system_mtime',
    'user_mtime',
    'jsonmodel_type',
  );
  $resolveref($data);
  $filter($data, $keys);
  return $data;
}

function islandora_aspace_create_datastreams($object, $data) {
  $data = islandora_aspace_preprocess_aspace_data($data);
  $json_data = json_encode($data);
  $json_data = islandora_aspace_jsonpp($json_data);

  if(!isset($object[ISLANDORA_ASPACE_DSID])) {
    $aspace_ds = $object->constructDatastream(ISLANDORA_ASPACE_DSID, 'M');
    $aspace_ds->label = 'ArchivesSpace JSON Data';
    $aspace_ds->mimetype = 'application/json';
    $aspace_ds->setContentFromString($json_data);
    $object->ingestDatastream($aspace_ds);
  }
  else {
    $object[ISLANDORA_ASPACE_DSID]->setContentFromString($json_data);
  }

  $mods_template = variable_get('islandora_aspace_mods_template', ISLANDORA_ASPACE_MODS);
  $mods_xml = token_replace($mods_template, array('islandora_aspace' => array($object, $data)));
  $mods_transform = variable_get('islandora_aspace_self_transform', 'No Transform');
  $mods_xml = islandora_aspace_transform_document($mods_xml, $mods_transform);

  if(!isset($object['MODS'])) {
    $mods_ds = $object->constructDatastream('MODS', 'M');
    $mods_ds->label = 'MODS Record';
    $mods_ds->mimetype = 'application/xml';
    $mods_ds->setContentFromString($mods_xml);
    $object->ingestDatastream($mods_ds);
  }
  else {
    $object['MODS']->setContentFromString($mods_xml);
  }

  $dc_transform = variable_get('islandora_aspace_transform', 'No Transform');
  if ($dc_transform != 'No Transform') {
    $dc_xml = islandora_aspace_transform_document($mods_xml, $dc_transform);
    if(!isset($object['DC'])) {
      $dc_ds = $object->constructDatastream('DC', 'X');
      $dc_ds->label = 'DC Record';
      $dc_ds->mimetype = 'application/xml';
      $dc_ds->setContentFromString($dc_xml);
      $object->ingestDatastream($dc_ds);
    }
    else {
      $object['DC']->setContentFromString($dc_xml);
    }
  }
}

function islandora_aspace_generate_token() {
  $length = 40;
  $token = base64_encode(openssl_random_pseudo_bytes($length));
  $token = rtrim($token, '=');
  $token = str_replace('+', 'A', $token);
  $token = str_replace('/', 'z', $token);
  return $token;
}

function islandora_aspace_check_identifier($repository_uri, $identifier) {
  module_load_include('inc', 'islandora_aspace', 'includes/archivesspace');
  $return = islandora_aspace_search_repository($repository_uri, "digital_object_id:{$identifier}");
  if($return->code != 200) {
    return $return;
  }

  $data = json_decode($return->data, TRUE);
  if(isset($data['total_hits']) && $data['total_hits'] == 0) {
    return FALSE;
  }
  else {
    return TRUE;
  }
}

/**
 * jsonpp - Pretty print JSON data
 *
 * In versions of PHP < 5.4.x, the json_encode() function does not yet provide a
 * pretty-print option. In lieu of forgoing the feature, an additional call can
 * be made to this function, passing in JSON text, and (optionally) a string to
 * be used for indentation.
 *
 * This can be removed and replaced with the native PHP call once Islandora stops
 * supporting PHP 5.3.3.
 *
 * @param string $json  The JSON data, pre-encoded
 * @param string $istr  The indentation string
 *
 * @link https://github.com/ryanuber/projects/blob/master/PHP/JSON/jsonpp.php
 *
 * @return string
 */
function islandora_aspace_jsonpp($json, $istr='  ')
{
    $result = '';
    for($p=$q=$i=0; isset($json[$p]); $p++)
    {
        $json[$p] == '"' && ($p>0?$json[$p-1]:'') != '\\' && $q=!$q;
        if(!$q && strchr(" \t\n\r", $json[$p])){continue;}
        if(strchr('}]', $json[$p]) && !$q && $i--)
        {
            strchr('{[', $json[$p-1]) || $result .= "\n".str_repeat($istr, $i);
        }
        $result .= $json[$p];
        if(strchr(',{[', $json[$p]) && !$q)
        {
            $i += strchr('{[', $json[$p])===FALSE?0:1;
            strchr('}]', $json[$p+1]) || $result .= "\n".str_repeat($istr, $i);
        }
    }
    return $result;
}
