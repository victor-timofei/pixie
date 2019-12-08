import './App.scss';

import {cloudGQLClient} from 'common/cloud-gql-client';
import {VersionInfo} from 'components/version-info/version-info';
import {Login} from 'containers/login';
import * as React from 'react';
import {ApolloProvider} from 'react-apollo';
import {Route, Router, Switch, withRouter} from 'react-router-dom';
import {isProd, PIXIE_CLOUD_VERSION} from 'utils/env';
import history from 'utils/pl-history';

export interface AppProps {
  name: string;
}

export class App extends React.Component<AppProps, {}> {
  render() {
    return (
      <>
        <Router history={history}>
          <ApolloProvider client={cloudGQLClient}>
            <div className='main-page'>
              <div className='content'>
                <Switch>
                  <Route exact path='/create' component={Login} />
                  <Route exact path='/auth_success' component={Login} />
                  <Route component={Login} />
                </Switch>
              </div>
            </div>
          </ApolloProvider>
        </Router>
        {!isProd() ? <VersionInfo /> : null}
      </>
    );
  }
}

export default withRouter(App);
