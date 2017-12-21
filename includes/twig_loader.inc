<?php

/**
 * This Twig_LoaderInterface is used to load templates from drupal variables.
 */
class IslandoraAspaceVariableTwigLoader implements Twig_LoaderInterface, Twig_ExistsLoaderInterface, Twig_SourceContextLoaderInterface {

  /**
   * Returns the source for a given template name.
   */
  public function getSource($name) {
    if (FALSE === $data = self::get($name)) {
      throw new Twig_Error_Loader(sprintf('Template "%s" does not exist.', $name));
    }

    return $data['source'];
  }

  /**
   * Returns the twig source for a given template name.
   */
  public function getSourceContext($name) {
    return new Twig_Source($this->getSource($name), $name);
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
   * @param string $machine_name
   *   The machine name of the template to save.
   * @param string $display_name
   *   The display name of the template to save.
   * @param string $source
   *   The template to be saved.
   */
  public static function set($machine_name, $display_name, $source) {
    $templates = variable_get('islandora_aspace_templates', array());
    $templates[$machine_name] = array(
      'name' => $display_name,
      'source' => $source,
      'last_modified' => time(),
      'machine_name' => $machine_name,
    );
    variable_set('islandora_aspace_templates', $templates);
  }

  /**
   * Remove a template from this loader.
   */
  public static function remove($machine_name) {
    $templates = variable_get('islandora_aspace_templates', array());
    unset($templates[$machine_name]);
    variable_set('islandora_aspace_templates', $templates);
  }

  /**
   * Lists all templates registered with this loader.
   */
  public static function getAll() {
    return variable_get('islandora_aspace_templates', array());
  }

}
