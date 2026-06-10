import {themes as prismThemes} from 'prism-react-renderer';
import type {Config} from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';

// This runs in Node.js - Don't use client-side code here (browser APIs, JSX...)

const config: Config = {
  title: 'Cleaf',
  tagline: 'A small, statically typed compiled language for x86-64 Linux',
  favicon: 'img/favicon.ico',

  future: {
    v4: true,
  },

  url: 'https://viastolfi.github.io',
  baseUrl: '/cleaf/',

  organizationName: 'viastolfi',
  projectName: 'cleaf',

  onBrokenLinks: 'throw',

  // Even if you don't use internationalization, you can use this field to set
  // useful metadata like html lang. For example, if your site is Chinese, you
  // may want to replace "en" with "zh-Hans".
  i18n: {
    defaultLocale: 'en',
    locales: ['en'],
  },

  presets: [
    [
      'classic',
      {
        docs: {
          sidebarPath: './sidebars.ts',
          editUrl: 'https://github.com/viastolfi/cleaf/tree/main/docs/',
        },
        blog: false,
        theme: {
          customCss: './src/css/custom.css',
        },
      } satisfies Preset.Options,
    ],
  ],

  themeConfig: {
    // Replace with your project's social card
    image: 'img/docusaurus-social-card.jpg',
    colorMode: {
      respectPrefersColorScheme: true,
    },
    navbar: {
      title: 'Cleaf',
      logo: {
        alt: 'Cleaf Logo',
        src: 'img/logo.svg',
      },
      items: [
        {
          type: 'docSidebar',
          sidebarId: 'langSidebar',
          position: 'left',
          label: 'Docs',
        },
        {
          href: 'https://github.com/viastolfi/cleaf',
          label: 'GitHub',
          position: 'right',
        },
      ],
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'Docs',
          items: [
            { label: 'Introduction', to: '/docs/intro' },
            { label: 'Getting Started', to: '/docs/getting-started' },
          ],
        },
        {
          title: 'More',
          items: [
            {
              label: 'GitHub',
              href: 'https://github.com/viastolfi/cleaf',
            },
          ],
        },
      ],
      copyright: `Copyright © ${new Date().getFullYear()} Cleaf. Built with Docusaurus.`,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
    },
  } satisfies Preset.ThemeConfig,
};

export default config;
