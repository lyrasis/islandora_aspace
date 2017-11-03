<?php

/**
 * @file
 * This file documents all available hook functions to manipulate data.
 */

/**
 * Paths for islandora_aspace to look for twig templates.
 *
 * @return string|array
 *   An array or a string containing path(s) to search for files
 *   ending in .twig to use as templates for the aspace data.
 */
function hook_islandora_aspace_template_paths() {
  $module_path = drupal_get_path('module', 'islandora_aspace');
  return "${module_path}/templates";
}
