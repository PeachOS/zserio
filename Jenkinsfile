pipeline {
  agent {
    node {
      label 'linux64'
    }

  }
  stages {
    stage('Start') {
      agent {
        node {
          label 'linux64'
        }

      }
      steps {
        echo 'Starting pipeline'
      }
    }
    stage('Build linux64') {
      steps {
        echo 'Build linux64'
      }
    }
  }
}