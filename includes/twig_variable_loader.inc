<?php

/**
 * This Twig_LoaderInterface is used to load templates from drupal variables.
 */
class IslandoraAspaceVariableTwigLoader implements Twig_LoaderInterface {

  /**
   * Returns the twig source for a given template name.
   */
  public function getSourceContext($name) {
    if (FALSE === $data = self::get($name)) {
      throw new Twig_Error_Loader(sprintf('Template "%s" does not exist.', $name));
    }

    return new Twig_Source($data['source'], $name);
  }

  /**
   * Checks if a given template name exists.
   */
  public function exists($name) {
    return FALSE !== self::get($name);
  }

  /**
   * Gets the cache key for a template name.
   */
  public function getCacheKey($name) {
    return $name;
  }

  /**
   * Checks if the template is fresh or should be rebuilt.
   */
  public function isFresh($name, $time) {
    if (FALSE === $data = self::get($name)) {
      throw new Twig_Error_Loader(sprintf('Template "%s" does not exist.', $name));
    }

    return $data['last_modified'] <= $time;
  }

  /**
   * Used to load a template array with variable_get.
   *
   * @param string $name
   *   Name of the template to load.
   *
   * @return array
   *   An array containing the keys:
   *     - source - a string containing the template
   *     - last_modified - a timestamp of the last modification to the template.
   */
  public static function get($name) {
    $templates = variable_get('islandora_aspace_templates', array());
    return isset($templates[$name]) ? $templates[$name] : FALSE;
  }

  /**
   * Used to save a template with variable_set.
   *
   * @param string $name
   *   The name of the template to save.
   * @param string $source
   *   The template to be saved.
   */
  public static function set($name, $source) {
    $templates = variable_get('islandora_aspace_templates', array());
    $templates[$name] = array(
      'source' => $source,
      'last_modified' => time(),
    );
    variable_set('islandora_aspace_templates', $templates);
  }

  /**
   * Lists all templates registered with this loader.
   */
  public static function list() {
    $templates = variable_get('islandora_aspace_templates', array());
    return array_keys($templates);
  }

}
