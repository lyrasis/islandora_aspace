<?php

class AspaceBatchImporter Extends ZipBatchImporter {

  protected $itemClass = 'AspaceImportObject';

  public static function getForm(array &$form_state) {
    module_load_include('inc', 'islandora_aspace', 'includes/archivesspace');

    $form = parent::getForm($form_state);
    $cm_options = $form['fs']['fs']['content_model']['#options'];

    // Filter content models based on what is enabled for Aspace
    $aspace_models = variable_get('islandora_aspace_enabled_models');
    $cm_options = array_intersect_key($cm_options, array_flip($aspace_models));

    $form['fs']['fs']['content_model']['#options'] = $cm_options;

    $repositories = islandora_aspace_get_repositories();
    $options = array();
    if($repositories !== FALSE) {
      foreach($repositories as $repository) {
        $options[$repository->uri] = $repository->name;
      }
    }
    else {
      drupal_set_message('Error contacting ArchivesSpace server. Please try again later.', 'error');
    }

    $form['fs']['aspace'] = array(
      '#type' => 'fieldset',
      '#title' => t('Archivesspace'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );

    $form['fs']['aspace']['islandora_aspace_published'] = array(
      '#type' => 'checkbox',
      '#title' => t('Publish'),
      '#description' => 'Should the item be published on ingest?',
    );

    $form['fs']['aspace']['islandora_aspace_repository'] = array(
      '#type' => 'select',
      '#title' => t('Select Repository'),
      '#description' => t('Choose which ArchivesSpace repository to add this object to.'),
      '#options' => $options,
      '#required' => TRUE,
    );

    unset($form['fs']['#description']);
    $form['fs']['submit']['#weight'] = 50;

    return $form;
  }

  public static function getBatchInfo(array &$form_state) {
    $info = parent::getBatchInfo($form_state);
    $info['published'] = $form_state['values']['islandora_aspace_published'];
    $info['repository'] = $form_state['values']['islandora_aspace_repository'];
    return $info;
  }
}

class AspaceImportObject Extends ZipBatchImportObject {
  protected $repository;
  protected $published;

  public static function getOne(&$info) {
    $object = parent::getOne($info);
    $object->repository = $info['repository'];
    $object->published = $info['published'];
    return $object;
  }

  public function getWrapperClass() {
    return 'AspaceBatchObject';
  }

  public function getRepository() {
    return $this->repository;
  }

  public function getPublished() {
    return $this->published;
  }
}

class AspaceBatchObject Extends IslandoraImporterBatchObject {

  public function batchProcess() {
    module_load_include('inc', 'islandora_aspace', 'includes/utilities');
    module_load_include('inc', 'islandora_aspace', 'includes/archivesspace');
    $parent_status = parent::batchProcess();
    if ($parent_status != ISLANDORA_BATCH_STATE__DONE) {
      return $parent_status;
    }

    $return = islandora_aspace_create_deposit(
      $this->importerObjectInstance->getRepository(),
      $this->label,
      $this->id,
      $this->importerObjectInstance->getPublished(),
      $this->id
    );

    $body = json_decode($return->data, TRUE);
    if ($return->code != 200) {
      return ISLANDORA_BATCH_STATE__ERROR;
    }

    $models = $this->models;
    $models[] = ISLANDORA_ASPACE_OBJECT_CMODEL;
    $this->models = $models;
    $this->state = $published ? 'A' : 'I';
    islandora_aspace_create_datastreams($this, $body);

    return ISLANDORA_BATCH_STATE__DONE;
  }
}
