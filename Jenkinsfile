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
        sh 'ls -la'
      }
    }
    stage('Build linux64') {
      agent {
        node {
          label 'linux64'
        }

      }
      steps {
        echo 'Build linux64'
        sh 'ls 3rdparty'
      }
    }
  }
}