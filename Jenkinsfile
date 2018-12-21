pipeline {
  agent {
    node {
      label 'linux64'
    }

  }
  stages {
    stage('checkout') {
      agent {
        node {
          label 'linux64'
        }

      }
      steps {
        echo 'Checkout'
      }
    }
  }
}