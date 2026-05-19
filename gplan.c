<!DOCTYPE html>
<html class="html-devise-layout gl-system" lang="en">
<head prefix="og: http://ogp.me/ns#">
<meta charset="utf-8">
<meta content="IE=edge" http-equiv="X-UA-Compatible">
<meta content="width=device-width, initial-scale=1" name="viewport">
<title>Sign in · GitLab</title>
<script nonce="BjZLohpjbdTT9qAXDVj/OA==">
//<![CDATA[
window.gon={};gon.features={"passkeys":true,"twoStepSignIn":false};
//]]>
</script>

<script nonce="BjZLohpjbdTT9qAXDVj/OA==">
//<![CDATA[
const root = document.documentElement;
if (window.matchMedia('(prefers-color-scheme: dark)').matches) {
  root.classList.add('gl-dark');
}

window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', (e) => {
  if (e.matches) {
    root.classList.add('gl-dark');
  } else {
    root.classList.remove('gl-dark');
  }
});

//]]>
</script>




<meta content="light dark" name="color-scheme">
<link rel="stylesheet" href="/assets/application-6cfc5bf1accc7be60a097014ba8b526bd361cfa9f5b0755ec6c38716971f3f05.css" media="(prefers-color-scheme: light)" />
<link rel="stylesheet" href="/assets/application_dark-78fb9312ecec789a0e5325c913c3401bf8b9ca4dac842db0da7f0127028c6117.css" media="(prefers-color-scheme: dark)" />
<link rel="stylesheet" href="/assets/page_bundles/login-7240ec00cf3969b710fe5e8959f8ef8eeff66d635ded28839b1b4256ae8d96a3.css" /><link rel="stylesheet" href="/assets/page_bundles/commit_description-9e7efe20f0cef17d0606edabfad0418e9eb224aaeaa2dae32c817060fa60abcc.css" /><link rel="stylesheet" href="/assets/page_bundles/work_items-fe78a3dcdbc87456d93535d15aa5ef609b9a8623591a687b335e85fa08fdbd12.css" /><link rel="stylesheet" href="/assets/page_bundles/notes_shared-e58f07f39b2c4557134baf9f32b68c875fffb37fe1c5d8808fb7607d19f5690b.css" />
<link rel="stylesheet" href="/assets/tailwind_cqs-ef4824f60d56559339ee7cfc71e29c445ebea667adc766d6ee1f6eb3bfc9f2bc.css" />


<link rel="stylesheet" href="/assets/fonts-deb7ad1d55ca77c0172d8538d53442af63604ff490c74acc2859db295c125bdb.css" />
<link rel="stylesheet" href="/assets/highlight/themes/white-91053d9b33e6ca83f1ccc9f675156756bb42617c9bc1cda4b3915abc0547ceda.css" media="(prefers-color-scheme: light)" />
<link rel="stylesheet" href="/assets/highlight/themes/dark-fa67ecd7679c9e378a28d89e0ab7f2423d7dbba3770b71f213b0453730ea247e.css" media="(prefers-color-scheme: dark)" />

<script src="/assets/webpack/runtime.a11a4e60.bundle.js" defer="defer" nonce="BjZLohpjbdTT9qAXDVj/OA=="></script>
<script src="/assets/webpack/main.e0c47439.chunk.js" defer="defer" nonce="BjZLohpjbdTT9qAXDVj/OA=="></script>
<script src="/assets/webpack/tracker.1eb446cb.chunk.js" defer="defer" nonce="BjZLohpjbdTT9qAXDVj/OA=="></script>
<script nonce="BjZLohpjbdTT9qAXDVj/OA==">
//<![CDATA[
window.snowplowOptions = {"namespace":"gl","hostname":"salsa.debian.org:443","postPath":"/-/collect_events","forceSecureTracker":true,"appId":"gitlab_sm"};
gl = window.gl || {};
gl.snowplowStandardContext = {"schema":"iglu:com.gitlab/gitlab_standard/jsonschema/1-1-8","data":{"environment":"self-managed","source":"gitlab-rails","correlation_id":"01KRZCV7VZ992SHX1N6T7SR3Z4","extra":{},"user_id":null,"global_user_id":null,"user_type":null,"is_gitlab_team_member":null,"namespace_id":null,"ultimate_parent_namespace_id":null,"project_id":null,"feature_enabled_by_namespace_ids":null,"realm":"self-managed","deployment_type":"self-managed","context_generated_at":"2026-05-19T05:55:23.696Z","organization_id":1}};
gl.snowplowPseudonymizedPageUrl = "https://salsa.debian.org/users/sign_in";
gl.maskedDefaultReferrerUrl = null;
gl.ga4MeasurementId = 'G-ENFH3X7M5Y';
gl.duoEvents = [];
gl.onlySendDuoEvents = false;


//]]>
</script>
<link rel="preload" href="/assets/application-6cfc5bf1accc7be60a097014ba8b526bd361cfa9f5b0755ec6c38716971f3f05.css" as="style" type="text/css" nonce="Q1pu7/PmxqDDzoIHjSWneg==">
<link rel="preload" href="/assets/highlight/themes/white-91053d9b33e6ca83f1ccc9f675156756bb42617c9bc1cda4b3915abc0547ceda.css" as="style" type="text/css" nonce="Q1pu7/PmxqDDzoIHjSWneg==">




<script src="/assets/webpack/commons-pages.explore.catalog-pages.groups.harbor.repositories-pages.groups.issues-pages.groups.new--aa29c505.0ea1bdf9.chunk.js" defer="defer" nonce="BjZLohpjbdTT9qAXDVj/OA=="></script>
<script src="/assets/webpack/commons-pages.search.show-super_sidebar.3d3a4264.chunk.js" defer="defer" nonce="BjZLohpjbdTT9qAXDVj/OA=="></script>
<script src="/assets/webpack/super_sidebar.5c142ba2.chunk.js" defer="defer" nonce="BjZLohpjbdTT9qAXDVj/OA=="></script>
<script src="/assets/webpack/commons-pages.admin.sessions-pages.ldap.omniauth_callbacks-pages.omniauth_callbacks-pages.profiles.p-27bfc84e.cfe4b2b4.chunk.js" defer="defer" nonce="BjZLohpjbdTT9qAXDVj/OA=="></script>
<script src="/assets/webpack/commons-pages.admin.sessions-pages.ldap.omniauth_callbacks-pages.omniauth_callbacks-pages.sessions-p-ea3be603.58b2db9e.chunk.js" defer="defer" nonce="BjZLohpjbdTT9qAXDVj/OA=="></script>
<script src="/assets/webpack/commons-pages.registrations.new-pages.sessions.new.d9de2f1d.chunk.js" defer="defer" nonce="BjZLohpjbdTT9qAXDVj/OA=="></script>
<script src="/assets/webpack/pages.sessions.new.431d3702.chunk.js" defer="defer" nonce="BjZLohpjbdTT9qAXDVj/OA=="></script>



<meta content="object" property="og:type">
<meta content="GitLab" property="og:site_name">
<meta content="Sign in · GitLab" property="og:title">
<meta content="Debian Salsa Gitlab" property="og:description">
<meta content="https://salsa.debian.org/assets/twitter_card-570ddb06edf56a2312253c5872489847a0f385112ddbcd71ccfa1570febab5d2.jpg" property="og:image">
<meta content="64" property="og:image:width">
<meta content="64" property="og:image:height">
<meta content="https://salsa.debian.org/users/sign_in" property="og:url">
<meta content="summary" property="twitter:card">
<meta content="Sign in · GitLab" property="twitter:title">
<meta content="Debian Salsa Gitlab" property="twitter:description">
<meta content="https://salsa.debian.org/assets/twitter_card-570ddb06edf56a2312253c5872489847a0f385112ddbcd71ccfa1570febab5d2.jpg" property="twitter:image">

<meta name="csrf-param" content="authenticity_token" />
<meta name="csrf-token" content="LoWRqrEGahvw_lBse-YE0F8J5w2MLFnKkFpqe-12YamH7mrDMc0wOqTyOhh3Q73CEikUrRdp9BiKjucc_DxN2A" />
<meta name="csp-nonce" content="BjZLohpjbdTT9qAXDVj/OA==" />
<meta name="action-cable-url" content="/-/cable" />
<link href="/-/manifest.json" rel="manifest">
<link rel="icon" type="image/png" href="/uploads/-/system/appearance/favicon/1/gitlabian_32.png" id="favicon" data-original-href="/uploads/-/system/appearance/favicon/1/gitlabian_32.png" />
<link rel="apple-touch-icon" type="image/x-icon" href="/assets/apple-touch-icon-b049d4bc0dd9626f31db825d61880737befc7835982586d015bded10b4435460.png" />
<link href="/search/opensearch.xml" rel="search" title="Search GitLab" type="application/opensearchdescription+xml">




<meta content="Debian Salsa Gitlab" name="description">
<meta content="#F1F0F6" media="(prefers-color-scheme: light)" name="theme-color">
<meta content="#232128" media="(prefers-color-scheme: dark)" name="theme-color">
</head>

<body class="gl-h-full login-page gl-browser-generic gl-platform-other" data-page="sessions:new" data-testid="login-page">

<script nonce="BjZLohpjbdTT9qAXDVj/OA==">
//<![CDATA[
gl = window.gl || {};
gl.client = {"isGeneric":true,"isOther":true};


//]]>
</script>




<div class="gl-h-full gl-flex gl-flex-wrap">
<div class="container gl-self-center">
<main class="content">
<div class="flash-container flash-container-page sticky" data-testid="flash-container">
<div id="js-global-alerts"></div>
</div>

<div class="row gl-mt-5 gl-gap-y-6">
<div class="col-md order-12">
<div class="col-sm-12">
<h1 class="gl-mb-5 gl-text-size-h2 gl-hidden md:gl-block">
Debian Salsa Gitlab
</h1>
<div class="md" id="js-custom-sign-in-description"><p data-sourcepos="1:1-1:73" dir="auto">Sign in to create issues, write comments, review contributions, and more.</p>&#x000A;<p data-sourcepos="3:1-3:259" dir="auto"><strong data-sourcepos="3:1-3:48">New user registrations are manually reviewed</strong> due to the high volume of spam registrations. It may take a few days before a new account is approved.  More information in the <a data-sourcepos="3:178-3:258" href="https://wiki.debian.org/Salsa/FAQ#User_registration" rel="nofollow noreferrer noopener" target="_blank">Frequently Asked Questions</a>.</p>&#x000A;<p data-sourcepos="5:1-5:125" dir="auto">You can also use our <a data-sourcepos="5:22-5:79" href="https://signup.salsa.debian.org/" rel="nofollow noreferrer noopener" target="_blank">self-service interface</a> to create a team group for managing projects.</p>&#x000A;<p data-sourcepos="7:1-7:67" dir="auto">If you need support check <a href="https://wiki.debian.org/Salsa/Doc#Support" rel="nofollow noreferrer noopener" target="_blank">https://wiki.debian.org/Salsa/Doc#Support</a></p></div>
</div>
</div>
<div class="col-md order-md-12">
<div class="col-sm-12 bar">
<div class="gl-text-center gl-mb-5">
<img alt="Debian Salsa Gitlab" class="gl-invisible gl-h-10 js-portrait-logo-detection lazy" data-src="/assets/logo-911de323fa0def29aaf817fca33916653fc92f3ff31647ac41d2c39bbe243edb.svg" src="data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==" />
<h1 class="gl-mb-3 gl-text-size-h2 md:gl-hidden">
Debian Salsa Gitlab
</h1>
</div>

<div class="js-non-oauth-login">
<div data-app-data="{&quot;sign_in_path&quot;:&quot;/users/sign_in&quot;,&quot;users_sign_in_path_path&quot;:&quot;/users/sign_in_path&quot;,&quot;passkeys_sign_in_path&quot;:&quot;/users/passkeys/sign_in&quot;,&quot;is_unconfirmed_email&quot;:false,&quot;new_user_confirmation_path&quot;:&quot;/users/confirmation/new&quot;,&quot;new_password_path&quot;:&quot;/users/password/new&quot;,&quot;show_captcha&quot;:false,&quot;is_remember_me_enabled&quot;:true}" id="js-sign-in-form">
<div class="gl-spinner-container gl-my-5" role="status"><span aria-hidden class="gl-spinner gl-spinner-md gl-spinner-dark !gl-align-text-bottom"></span><span class="gl-sr-only !gl-absolute">Loading</span>
</div>
<form action="/users/sign_in" accept-charset="UTF-8" method="post"><input type="hidden" name="authenticity_token" value="NuqCYKdWcmtxce-XXtqpH3iKYPlF6_TJ7aVgiwdla3KfgXkJJ50oSiV9heNSfxANNaqTWd6uWRv3ce3sFi9HAw" autocomplete="off" /><input data-js-name="login" autocomplete="off" type="hidden" name="user[login]" id="user_login" />
<input data-js-name="password" autocomplete="off" type="hidden" name="user[password]" id="user_password" />
<input data-js-name="rememberMe" autocomplete="off" type="hidden" name="user[remember_me]" id="user_remember_me" />
</form></div>

</div>
<div class="gl-mt-3 gl-text-center">
Don&#39;t have an account yet?
<a data-testid="register-link" data-track-action="click_register_from_sign_in_page" href="/users/sign_up">Register now</a>
</div>
<div class="gl-flex gl-items-center gl-gap-5" data-testid="divider">
<hr class="gl-grow gl-border-default">
or sign in with
<hr class="gl-grow gl-border-default">
</div>

<div class="gl-mt-5 gl-text-center gl-flex gl-flex-col gl-gap-3 js-oauth-login">
<form class="js-omniauth-form" method="post" action="/users/auth/google_oauth2"><button class="gl-button btn btn-block btn-md btn-default " type="submit"><span class="gl-button-text">
<img alt="Google" title="Sign in with Google" class="gl-button-icon lazy" data-src="/assets/auth_buttons/google_64-9ab7462cd2115e11f80171018d8c39bd493fc375e83202fbb6d37a487ad01908.png" src="data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==" />
<span class="gl-button-text">
Google
</span>

</span>

</button><input type="hidden" name="authenticity_token" value="i8udP5wgaAG9fhoDNls6_ABagsIud1fsFGDMhTjTDx4ioGZWHOsyIOlycHc6_oPuTXpxYrUy-j4OtEHiKZkjbw" autocomplete="off" /></form>
<form class="js-omniauth-form" method="post" action="/users/auth/bitbucket"><button class="gl-button btn btn-block btn-md btn-default " type="submit"><span class="gl-button-text">
<img alt="Bitbucket" title="Sign in with Bitbucket" class="gl-button-icon lazy" data-src="/assets/auth_buttons/bitbucket_64-daa496030c0c290748e3c2e50f7464d2f5de0e019cce728930e0508a6dac815c.png" src="data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==" />
<span class="gl-button-text">
Bitbucket
</span>

</span>

</button><input type="hidden" name="authenticity_token" value="aFoj0WnTqmU-5Rl1mVLEsf0yMxXSY9TZm28FUs9mOGPBMdi46RjwRGrpcwGV932jsBLAtUkmeQuBu4g13iwUEg" autocomplete="off" /></form>
<form class="js-omniauth-form" method="post" action="/users/auth/gitlab"><button class="gl-button btn btn-block btn-md btn-default " data-testid="gitlab-oauth-login-button" type="submit"><span class="gl-button-text">
<img alt="GitLab.com" title="Sign in with GitLab.com" class="gl-button-icon lazy" data-src="/assets/auth_buttons/gitlab_64-2808a5fce900f4cb87b4dce9f9907bf68ac00a39086aa3a125e3250652f9967f.png" src="data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==" />
<span class="gl-button-text">
GitLab.com
</span>

</span>

</button><input type="hidden" name="authenticity_token" value="ULu0LI2M54KsjgTGSBfktQac2ARCjCicURgs9XsJu8b50E9FDUe9o_iCbrJEsl2nS7wrpNnJhU5LzKGSakOXtw" autocomplete="off" /></form>
<div class="gl-form-checkbox custom-control custom-checkbox">
<input type="checkbox" name="js-remember-me-omniauth" id="js-remember-me-omniauth" class="custom-control-input" />
<label class="custom-control-label" for="js-remember-me-omniauth"><span>Remember me
</span></label>
</div>
</div>


</div>
</div>
</div>
</main>
</div>
<div class="footer-container gl-w-full gl-self-end">
<hr class="gl-m-0">
<div class="container gl-py-5 gl-flex gl-justify-between gl-items-start">
<div class="gl-hidden md:gl-flex gl-gap-5 gl-flex-wrap">
<a href="/explore">Explore</a>
<a href="/help">Help</a>
<a href="https://about.gitlab.com">About GitLab</a>
<a target="_blank" class="text-nowrap" rel="noopener noreferrer" href="https://forum.gitlab.com">GitLab community forum</a>
</div>
<div class="js-language-switcher" data-locales="[{&quot;value&quot;:&quot;en&quot;,&quot;percentage&quot;:100,&quot;text&quot;:&quot;English&quot;},{&quot;value&quot;:&quot;ja&quot;,&quot;percentage&quot;:99,&quot;text&quot;:&quot;日本語&quot;},{&quot;value&quot;:&quot;ga_IE&quot;,&quot;percentage&quot;:99,&quot;text&quot;:&quot;Gaeilge&quot;},{&quot;value&quot;:&quot;pt_BR&quot;,&quot;percentage&quot;:98,&quot;text&quot;:&quot;português (Brasil)&quot;},{&quot;value&quot;:&quot;ko&quot;,&quot;percentage&quot;:98,&quot;text&quot;:&quot;한국어&quot;},{&quot;value&quot;:&quot;it&quot;,&quot;percentage&quot;:98,&quot;text&quot;:&quot;italiano&quot;},{&quot;value&quot;:&quot;fr&quot;,&quot;percentage&quot;:98,&quot;text&quot;:&quot;français&quot;},{&quot;value&quot;:&quot;es&quot;,&quot;percentage&quot;:98,&quot;text&quot;:&quot;español&quot;},{&quot;value&quot;:&quot;de&quot;,&quot;percentage&quot;:95,&quot;text&quot;:&quot;Deutsch&quot;},{&quot;value&quot;:&quot;zh_CN&quot;,&quot;percentage&quot;:93,&quot;text&quot;:&quot;简体中文&quot;}]"></div>

</div>
</div>


</div>
</body>
</html>
