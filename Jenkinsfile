pipeline {
  agent any
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