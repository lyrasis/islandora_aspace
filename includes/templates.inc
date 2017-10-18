<?php

/**
 * @file
 * This file contains utility functions for the template system.
 */

/**
 * Static loader to get a twig object for use in islandora_aspace.
 */
function islandora_aspace_twig() {
  module_load_include('inc', 'islandora_aspace', 'includes/twig_loader');
  $twig = &drupal_static(__FUNCTION__);
  if (!isset($twig)) {
    $variable_loader = new IslandoraAspaceVariableTwigLoader();
    $paths = module_invoke_all('islandora_aspace_template_paths');
    $file_loader = new Twig_Loader_Filesystem($paths);
    $loader = new Twig_Loader_Chain(array($file_loader, $variable_loader));
    $twig = new Twig_Environment($loader);
  }
  return $twig;
}

/**
 * Get list of templates returned from hook call.
 */
function islandora_aspace_hook_template_list() {
  $list = array();

  $paths = module_invoke_all('islandora_aspace_template_paths');
  foreach ($paths as $path) {
    $files = scandir($path);
    foreach ($files as $file) {
      if (is_file($path . DIRECTORY_SEPARATOR . $file)) {
        $file_exploded = explode('.', $file);
        $list[$file] = $file_exploded[0];
      }
    }
  }

  return $list;
}

/**
 * Get a list of possible twig templates.
 */
function islandora_aspace_template_list() {
  module_load_include('inc', 'islandora_aspace', 'includes/twig_loader');
  $list = array();

  $variable_templates = IslandoraAspaceVariableTwigLoader::getAll();
  foreach ($variable_templates as $machine_name => $info) {
    $list[$machine_name] = $info['name'];
  }

  return $list + islandora_aspace_hook_template_list();
}

/**
 * Mangles a template name to be suitable as a machine name.
 */
function islandora_aspace_template_machine_name($name) {
  $machine_name = preg_replace('/[^a-z0-9_]+/', '_', strtolower($name));
  return $machine_name;
}