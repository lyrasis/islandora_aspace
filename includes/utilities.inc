<?php

function islandora_archivespace_rest() {
  module_load_include('inc', 'islandora_rest', 'includes/utilities');

  $request_method = islandora_rest_get_request_method();
  $object_data = islandora_rest_get_request_parameters();

  $uri = $object_data['file_versions'][0]['file_uri'];
  $uri = explode('/',$uri);
  $pid = $uri[count($uri)-1];

  $object = islandora_object_load($pid);
  if(!$object) {
    print 'Error';
    exit;
  }

  $title = $object_data['title'];
  $published = $object_data['publish'];

  $object->label = $title;
  $object->state = $published ? 'A' : 'I';
  print 'Success';
  exit();
}

function islandora_archivespace_get_repositories() {
  // TODO error handling.
  $connection = new CurlConnection();
  $repositories = $connection->getRequest(variable_get('islandora_archivespace_baseurl') . '/repositories');
  return json_decode($repositories['content']);
}

function islandora_archivespace_create_digital_object($title, $identifier, $repository_uri, $pid) {
  // TODO error handling.
  $connection = new CurlConnection();

  // authenticate
  $username = variable_get('islandora_archivespace_username');
  $password = variable_get('islandora_archivespace_password');
  $url = variable_get('islandora_archivespace_baseurl') . '/users/' . urlencode($username) . '/login?password=' . urlencode($password);
  $session_info = $connection->postRequest($url);
  $session_decoded = json_decode($session_info['content']);
  $session_key = $session_decoded->session;
  $uri = $GLOBALS['base_url'] . '/islandora/object/' . $pid;

  // make request
  $json = Array();
  $json['title'] = $title;
  $json['digital_object_id'] = $identifier;
  $json['file_versions'] = array(
    array(
      'file_uri' => $uri,
    ),
  );
  $data = json_encode($json);
  $url = variable_get('islandora_archivespace_baseurl') . $repository_uri . '/digital_objects';

  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'POST');
  curl_setopt($ch, CURLOPT_POST, TRUE);
  curl_setopt($ch, CURLOPT_HTTPHEADER, array("X-ArchivesSpace-Session: $session_key"));
  curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_VERBOSE, 1);
  curl_setopt($ch, CURLOPT_HEADER, 1);
  $curl_response = curl_exec($ch);
  $info = curl_getinfo($ch);
  $content = substr($curl_response, $info['header_size']);
}
